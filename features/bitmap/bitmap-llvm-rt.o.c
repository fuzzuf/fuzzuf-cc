// Orig: https://github.com/mboehme/aflfast/blob/master/llvm_mode/afl-llvm-rt.o.c
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

#include "config.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>

/* Globals needed by the injected instrumentation. The __bitmap_area_initial region
   is used for instrumentation output before __bitmap_map_shm() has a chance to run.
   It will end up as .comm, so it shouldn't be too wasteful. */

u8  __bitmap_area_initial[MAP_SIZE];
u8* __bitmap_area_ptr = __bitmap_area_initial;

__thread u32 __bitmap_prev_loc;


/* SHM setup. */

static void __bitmap_map_shm(void) {

  char *id_str = getenv(SHM_ENV_VAR);

  /* If we're running under AFL, attach to the appropriate region, replacing the
     early-stage __bitmap_area_initial region that is needed to allow some really
     hacky .init code to work correctly in projects such as OpenSSL. */

  if (id_str) {

    u32 shm_id = atoi(id_str);

    __bitmap_area_ptr = shmat(shm_id, NULL, 0);

    /* Whooooops. */

    if (__bitmap_area_ptr == (void *)-1) _exit(1);

    /* Write something into the bitmap so that even with low AFL_INST_RATIO,
       our parent doesn't give up on us. */

    __bitmap_area_ptr[0] = 1;

  }

}


/* This one can be called from user code when deferred forkserver mode
    is enabled. */

void __bitmap_manual_init(void) {

  static u8 init_done;

  if (!init_done) {

    __bitmap_map_shm();
    init_done = 1;

  }

}


/* Proper initialization routine. */

#if 9 < __GNUC__ || (__GNUC__ == 9 && 1 <= __GNUC_MINOR__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wprio-ctor-dtor"
__attribute__((constructor(0))) void __bitmap_auto_init(void) {
#pragma GCC diagnostic pop
#else
__attribute__((constructor(101))) void __bitmap_auto_init(void) {
#endif


  if (getenv(DEFER_ENV_VAR)) return;

  __bitmap_manual_init();

}
