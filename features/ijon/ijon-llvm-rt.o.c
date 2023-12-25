// Orig:
// https://github.com/RUB-SysSec/ijon/blob/master/llvm_mode/afl-llvm-rt.o.c
/*
   american fuzzy lop - LLVM instrumentation bootstrap
   ---------------------------------------------------

   Written by Laszlo Szekeres <lszekeres@google.com> and
              Michal Zalewski <lcamtuf@google.com>

   LLVM integration design comes from Laszlo Szekeres.

   Copyright 2015, 2016 Google Inc. All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at:

     http://www.apache.org/licenses/LICENSE-2.0

   This code is the rewrite of afl-as.h's main_payload.

*/

#include <assert.h>
#include <execinfo.h>
#include <signal.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "ijon.h"
#include "types.h"

/* This is a somewhat ugly hack for the experimental 'trace-pc-guard' mode.
   Basically, we need to make sure that the forkserver is initialized after
   the LLVM-generated runtime initialization pass, not before. */

#ifdef USE_TRACE_PC
#define CONST_PRIO 5
#else
#define CONST_PRIO 0
#endif /* ^USE_TRACE_PC */

/* Globals needed by the injected instrumentation. The __ijon_area_initial region
   is used for instrumentation output before __afl_map_shm() has a chance to
   run. It will end up as .comm, so it shouldn't be too wasteful. */

u8 __ijon_area_initial[MAP_SIZE];
u8 *__ijon_area_ptr = __ijon_area_initial;
u8 *__ijon_counter_ptr = __ijon_area_initial;

uint64_t __ijon_max_initial[MAXMAP_SIZE];
uint64_t *__ijon_max_ptr = __ijon_max_initial;

__thread u32 __ijon_state;
__thread u32 __ijon_state_log;
__thread u32 __ijon_mask = 0xffffffff;

static uint64_t afl_size = MAP_SIZE;
static uint64_t map_size = MAP_SIZE;
static uint64_t max_map_size = MAXMAP_SIZE;

void ijon_xor_state(uint32_t val) {
  __ijon_state = (__ijon_state ^ val) % map_size;
}

void ijon_push_state(uint32_t x) {
  ijon_xor_state(__ijon_state_log);
  __ijon_state_log = (__ijon_state_log << 8) | (x & 0xff);
  ijon_xor_state(__ijon_state_log);
}

void ijon_max(uint32_t addr, uint64_t val) {
  if (__ijon_max_ptr[addr % max_map_size] < val) {
    __ijon_max_ptr[addr % max_map_size] = val;
  }
}

void ijon_min(uint32_t addr, uint64_t val) {
  val = 0xffffffffffffffff - val;
  ijon_max(addr, val);
}

void ijon_map_inc(uint32_t addr) {
  __ijon_counter_ptr[(__ijon_state ^ addr) % map_size] += 1;
}

void ijon_map_set(uint32_t addr) {
  __ijon_counter_ptr[(__ijon_state ^ addr) % map_size] |= 1;
}

uint32_t ijon_strdist(char *a, char *b) {
  int i = 0;
  while (*a && *b && *a++ == *b++) {
    i++;
  }
  return i;
}

uint32_t ijon_memdist(char *a, char *b, size_t len) {
  size_t i = 0u;
  while (i < len && *a++ == *b++) {
    i++;
  }
  return i;
}

uint64_t ijon_simple_hash(uint64_t x) {
  x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
  x = x ^ (x >> 31);
  return x;
}

void ijon_enable_feedback() { __ijon_mask = 0xffffffff; }
void ijon_disable_feedback() { __ijon_mask = 0x0; }

uint32_t ijon_hashint(uint32_t old, uint32_t val) {
  uint64_t input = (((uint64_t)(old)) << 32) | ((uint64_t)(val));
  return (uint32_t)(ijon_simple_hash(input));
}
uint32_t ijon_hashstr(uint32_t old, char *val) {
  return ijon_hashmem(old, val, strlen(val));
}
uint32_t ijon_hashmem(uint32_t old, char *val, size_t len) {
  old = ijon_hashint(old, len);
  for (size_t i = 0; i < len; i++) {
    old = ijon_hashint(old, val[i]);
  }
  return old;
}

#ifdef __i386__
// WHY IS STACKUNWINDING NOT WORKING IN CGC BINARIES?
uint32_t ijon_hashstack_manual() {
  uint32_t *ebp = 0;
  uint64_t res = 0;
  asm("\t movl %%ebp,%0" : "=r"(ebp));
  for (int i = 0; i < 16 && ebp; i++) {
    // printf("ebp: %p\n", ebp);
    printf("ret: %x\n", ebp[1]);
    res ^= ijon_simple_hash((uint64_t)ebp[1]);
    ebp = (uint32_t *)ebp[0];
  }
  printf(">>>> Final Stackhash: %lx\n", res);
  return (uint32_t)res;
}
#endif

uint32_t ijon_hashstack_libgcc() {
  void *buffer[16] = {
      0,
  };
  int num = backtrace(buffer, 16);
  assert(num < 16);
  uint64_t res = 0;
  for (int i = 0; i < num; i++) {
    printf("stack_frame %p\n", buffer[i]);
    res ^= ijon_simple_hash((uint64_t)buffer[i]);
  }
  printf(">>>> Final Stackhash: %lx\n", res);
  return (uint32_t)res;
}

/* Running in persistent mode? */

// static u8 is_persistent;

/* SHM setup. */

static void __ijon_map_shm(void) {
  const char *afl_id_str = getenv(AFL_SHM_ENV_VAR);
  const char *afl_size_str = getenv(AFL_SIZE_ENV_VAR);
  const char *ijon_counter_size_str = getenv(IJON_COUNTER_SIZE_ENV_VAR);
  const char *ijon_max_size_str = getenv(IJON_MAX_SIZE_ENV_VAR);

  // Remap shared memory for IJON
  if (afl_id_str) {
    const char *afl_id_str_end = afl_id_str + strnlen(afl_id_str, 10);
    char *end = NULL;
    const u32 shm_id = strtol(afl_id_str, &end, 10);
    if (end != afl_id_str_end) {
      printf("Invalid value on " AFL_SHM_ENV_VAR);
      _exit(1);
    }
    u8 *cur = shmat(shm_id, NULL, 0);
    if (cur == (void *)-1) _exit(1);
    if (afl_size_str) {
      const char *afl_size_str_end = afl_size_str + strnlen(afl_size_str, 10);
      const u32 size = strtol(afl_size_str, &end, 10);
      if (end != afl_size_str_end) {
        printf("Invalid value on " AFL_SIZE_ENV_VAR);
        _exit(1);
      }
      if (size) {
        __ijon_area_ptr = cur;
        cur += size;
        afl_size = size;
      }
    }
    if (ijon_counter_size_str) {
      const char *ijon_counter_size_str_end =
          ijon_counter_size_str + strnlen(ijon_counter_size_str, 10);
      const u32 size = strtol(ijon_counter_size_str, &end, 10);
      if (end != ijon_counter_size_str_end) {
        printf("Invalid value on " IJON_COUNTER_SIZE_ENV_VAR);
        _exit(1);
      }
      if (size) {
        __ijon_counter_ptr = cur;
        cur += size;
        map_size = size;
      }
    }
    if (ijon_max_size_str) {
      const char *ijon_max_size_str_end =
          ijon_max_size_str + strnlen(ijon_max_size_str, 10);
      const u32 size = strtol(ijon_max_size_str, &end, 10);
      if (end != ijon_max_size_str_end) {
        printf("Invalid value on " IJON_MAX_SIZE_ENV_VAR);
        _exit(1);
      }
      if (size) {
        __ijon_max_ptr = (uint64_t *)cur;
        cur += size;
        max_map_size = size / sizeof(uint64_t);
      }
    }
    /* Whooooops. */

    /* Write something into the bitmap so that even with low AFL_INST_RATIO,
       our parent doesn't give up on us. */

    __ijon_area_ptr[0] = 1;
  }
}

/* This one can be called from user code when deferred forkserver mode
    is enabled. */

void __ijon_manual_init(void) {
  static u8 init_done;

  if (!init_done) {
    __ijon_map_shm();
    init_done = 1;
  }
}

/* Proper initialization routine. */

__attribute__((constructor)) void __ijon_auto_init(void) {
  printf("[*] __ijon_auto_init()\n");
  __ijon_manual_init();
}
