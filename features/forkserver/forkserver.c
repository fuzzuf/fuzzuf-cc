/*
 * fuzzuf-cc
 * Copyright (C) 2023 Ricerca Security
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

#define HANDSHAKE_FAILED ((uint64_t)-1)

FILE *log_file;

uint64_t Now() {
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  return spec.tv_sec * 1000 * 1000 + (spec.tv_nsec / 1000);  // in micro-seconds
}

#ifdef SHOW_LOG
void LogSuccess(const char *process_name, const char *format, ...) {
  va_list argptr;
  va_start(argptr, format);

  fprintf(log_file, "[*] [%lu:%s] ", Now(), process_name);
  vfprintf(log_file, format, argptr);

  va_end(argptr);
}
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void LogSuccess(const char *process_name, const char *format, ...) {
#pragma GCC diagnostic pop
  // Do nothing
}
#endif

void LogInfo(const char *process_name, const char *format, ...) {
  va_list argptr;
  va_start(argptr, format);

  fprintf(log_file, "[+] [%lu:%s] ", Now(), process_name);
  vfprintf(log_file, format, argptr);

  va_end(argptr);
}

void LogError(const char *process_name, const char *format, ...) {
  va_list argptr;

  va_start(argptr, format);
  fprintf(log_file, "[!] [%lu:%s] ", Now(), process_name);
  vfprintf(log_file, format, argptr);
  va_end(argptr);

  fflush(log_file);
}

pid_t put_pid;


typedef struct {
  int valid;
  pid_t self_pid;
  pid_t child_pid;
  int signal_number;
  int signal_code;
  int signal_status;
  int kill_errno;
} LastTimeout;

LastTimeout last_timeout;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void PUTTimeoutHandler(int sig, siginfo_t *si, void *ucontext) {
#pragma GCC diagnostic pop
  last_timeout.valid = 1;
  last_timeout.self_pid = getpid();
  last_timeout.child_pid = si->si_pid;
  last_timeout.signal_number = si->si_signo;
  last_timeout.signal_code = si->si_code;
  last_timeout.signal_status = si->si_status;

  // Remove existing alarm
  alarm(0);

  int err = kill(put_pid, SIGKILL);
  if (err == -1) {
    last_timeout.kill_errno = errno;
  }
  else {
    last_timeout.kill_errno = 0;
  }

  return;
  // Wait for SIGKILL
}

uint64_t WaitClientAttach() {
  uint64_t executor_id = 0;
  int nbytes = read(FORKSRV_FD_READ, &executor_id, sizeof(executor_id));
  if (nbytes < (int)sizeof(executor_id)) {
    perror("[!] [ForkServer] Failed to get executor id");
    fprintf(stderr, "\tTips: Is this forkserver attached to client?\n");
    fprintf(stderr, "\tJust executing program...\n");
    return HANDSHAKE_FAILED;
  }
  LogSuccess("ForkServer", "WaitClientAttach: executor_id=%d\n", executor_id);
  return executor_id;
}

void TellClientServerHasStarted() {
  pid_t self_pid = getpid();
  LogSuccess("ForkServer", "TellClientServerHasStarted: pid=%d\n", self_pid);
  int nbytes = write(FORKSRV_FD_WRITE, &self_pid, sizeof(self_pid));
  if (nbytes < (int)sizeof(self_pid)) {
    perror(
        "[!] [ForkServer] Failed to tell client that the server has started");
    exit(EXIT_FAILURE);
  }
  LogSuccess("ForkServer", "TellClientServerHasStarted: Done\n");
}

// Returns executor id
uint64_t DoHandshake() {
  uint64_t executor_id = WaitClientAttach();
  if (executor_id == HANDSHAKE_FAILED) {
    return HANDSHAKE_FAILED;
  }

  TellClientServerHasStarted();
  return executor_id;
}

int Recv(void *buf, size_t size) {
  size_t nread = 0;
  while (nread < size) {
    ssize_t res = read(FORKSRV_FD_READ, (char*)buf + nread, size - nread);
    if (res == -1) {
      if (errno == EINTR) {
        continue;
      }

      perror("[!] [ForkServer] Failed to read command");
      fprintf(stderr, "\tRequested to read %lu bytes, but read %lu bytes\n", size, res);
      fprintf(stderr, "\tTips: Is this forkserver attached to client?\n");
      exit(EXIT_FAILURE);
    }
    if (res == 0) {
      break;
    }
    nread += res;
  }
  return nread;
}

ForkServerAPI WaitForAPICall() {
  ForkServerAPI command = InvalidCommand;
  int nbytes = Recv(&command, sizeof(command));
  if (nbytes < (int)sizeof(command)) {
    perror("[!] [ForkServer] Failed on WaitForAPICall");
    fprintf(stderr, "\tRequested to read %lu bytes, but read %d bytes\n",
            sizeof(command), nbytes);
    exit(EXIT_FAILURE);
  }
  return command;
}

uint64_t RecvU64() {
  uint64_t response = 0;
  int nbytes = Recv(&response, sizeof(response));
  if (nbytes < (int)sizeof(response)) {
    perror("[!] [ForkServer] Failed on RecvU64()");
    fprintf(stderr, "\tRequested to read %lu bytes, but read %d bytes\n",
            sizeof(response), nbytes);
    exit(EXIT_FAILURE);
  }
  return response;
}

typedef struct {
  uint64_t executor_id;
  uint64_t timeout_us;
  bool read_stdin;
  bool save_stdout_stderr;
} RuntimeSettings;

void AttachStdxxxFiles(RuntimeSettings *settings) {
  if (settings->read_stdin) {
    {
      char path[MAX_FILE_PATH_LENGTH] = "";
      snprintf(path, sizeof(path), PREDEFINED_STDXXX_FILE_PATH,
               settings->executor_id, "stdin");
      LogSuccess("ForkServer", "stdin = %s\n", path);
      int put_stdin = open(path, O_RDONLY);
      if (put_stdin == -1) {
        perror("[!] [ForkServer] Failed to open stdin");
        exit(EXIT_FAILURE);
      }
      dup2(put_stdin, 0);
    }
    LogSuccess("ForkServer", "Attached stdin\n");
  }
  if (settings->save_stdout_stderr) {
    {
      char path[MAX_FILE_PATH_LENGTH] = "";
      snprintf(path, sizeof(path), PREDEFINED_STDXXX_FILE_PATH,
               settings->executor_id, "stdout");
      LogSuccess("ForkServer", "stdout = %s\n", path);
      int put_stdout = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
      if (put_stdout == -1) {
        perror("[!] [ForkServer] Failed to open stdout");
        exit(EXIT_FAILURE);
      }
      dup2(put_stdout, 1);
    }
    {
      char path[MAX_FILE_PATH_LENGTH] = "";
      snprintf(path, sizeof(path), PREDEFINED_STDXXX_FILE_PATH,
               settings->executor_id, "stderr");
      LogSuccess("ForkServer", "stderr = %s\n", path);
      int put_stderr = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
      if (put_stderr == -1) {
        perror("[!] [ForkServer] Failed to open stderr");
        exit(EXIT_FAILURE);
      }
      dup2(put_stderr, 2);
    }
    LogSuccess("ForkServer", "Attached stdout, stderr\n");
  }
}

bool ExecutePUT(RuntimeSettings *settings) {
#ifdef SHOW_LOG
  fflush(log_file);
#endif

  // Execute PUT
  put_pid = fork();
  if (put_pid < 0) {
    perror("[!] [ForkServer] Failed to fork process to spawn PUT");
    exit(EXIT_FAILURE);
  }
  if (put_pid == 0) {
    // In PUT process (child process)

    AttachStdxxxFiles(settings);

    // Close files which daemon (parent) process uses
    close(FORKSRV_FD_READ);
    close(FORKSRV_FD_WRITE);

    return true;
  }

  // In daemon process

  last_timeout.valid = 0;
  // Set timeout of PUT execution
  {
    struct itimerval _old_timeout;
    struct itimerval timeout = {
        .it_interval =
            {
                .tv_sec = 0,
                .tv_usec = 0,
            },
        .it_value = {
            .tv_sec = settings->timeout_us / (1000 * 1000),
            .tv_usec = settings->timeout_us % (1000 * 1000),
        }};
    int err = setitimer(ITIMER_REAL, &timeout, &_old_timeout);
    if (err == -1) {
      perror("[!] [Daemon] Failed to set timeout");
      exit(EXIT_FAILURE);
    }
  }
  {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = PUTTimeoutHandler;
    sa.sa_flags = SA_SIGINFO;
    int err = sigaction(SIGALRM, &sa, NULL);
    if (err == -1) {
      perror("[!] [Daemon] Failed to setup notification sender");
      exit(EXIT_FAILURE);
    }
  }

  // Wait for PUT exit
  // TODO: PUTがシグナルで **停止** した場合
  while (1) {
    int wstatus;
    pid_t pid = waitpid(put_pid, &wstatus, WUNTRACED);
    if( last_timeout.valid ) {
      LogSuccess(
        "Forkserver",
        "PUTTimeoutHandler(pid=%d): pid=%d, signo=%d, code=%d, status=%d\n",
        last_timeout.self_pid, last_timeout.child_pid, last_timeout.signal_number,
	last_timeout.signal_code, last_timeout.signal_status );
      if( last_timeout.kill_errno ) {
	if( last_timeout.kill_errno != ESRCH || pid != put_pid ) {
          LogError("Forkserver", "Failed to terminate PUT: errno=%d\n", last_timeout.kill_errno );
          perror("[!] [Daemon] Failed to terminate PUT");
	}
      }
      last_timeout.valid = 0;
    }
    if (pid == -1) {
      if (errno == EINTR) {
        LogSuccess("ForkServer", "Interrupted. Retrying waitpid()\n");
        continue;
      }
      perror("[!] [ForkServer] Failed to monitor PUT");
      exit(EXIT_FAILURE);
    }

    if (pid == put_pid) {
      ExecutePUTAPIResponse response = {
          .error = None, .exit_code = 0, .signal_number = 0};
      if (WIFEXITED(wstatus) | WIFSIGNALED(wstatus)) {
        if (WIFEXITED(wstatus)) {
          response.exit_code = WEXITSTATUS(wstatus);
          LogSuccess("ForkServer", "PUT(pid=%d): Exited: exit_code=%d\n",
                     put_pid, WEXITSTATUS(wstatus));
        }
        if (WIFSTOPPED(wstatus)) {
          LogSuccess("ForkServer", "PUT(pid=%d): Stopped: signal_number=%d\n",
                     put_pid, WSTOPSIG(wstatus));
        }
        if (WIFSIGNALED(wstatus)) {
          response.exit_code = WTERMSIG(wstatus);
          response.signal_number = WTERMSIG(wstatus);
          LogSuccess("ForkServer", "PUT(pid=%d): Signaled: signal_number=%d\n",
                     put_pid, WTERMSIG(wstatus));
        }
        {
          int nbytes = write(FORKSRV_FD_WRITE, &response, sizeof(response));
          if (nbytes == -1) {
            perror("[!] [ForkServer] Failed to tell fuzzer exit status of PUT");
            exit(EXIT_FAILURE);
          }
        }
        return false;
      }
    }
  }
}

__attribute__((constructor)) static void StartForkServer() {
  log_file = fopen("fuzzuf-cc.forkserver.log", "w");
  if (log_file == NULL) {
    perror("[!] Failed to open log file");
    exit(EXIT_FAILURE);
  }

  LogSuccess("ForkServer", "StartForkServer: Start\n");

  // Debug output
  LogSuccess("ForkServer", "__AFL_SHM_ID = %s\n", getenv("__AFL_SHM_ID"));

  // Default runtime settings
  RuntimeSettings settings = {
      .executor_id = 0,
      .timeout_us = 20 * 1000,  // [us]
      .read_stdin = false,
      .save_stdout_stderr = false,
  };

  uint64_t executor_id = DoHandshake();
  if (executor_id == HANDSHAKE_FAILED) {
    return;
  }

  settings.executor_id = executor_id;
  LogInfo("ForkServer", "StartForkServer: executor id is %lu\n",
          settings.executor_id);

  while (1) {
    LogSuccess("ForkServer", "StartForkServer: Waiting loop\n");

    ForkServerAPI command = WaitForAPICall();
    LogSuccess("ForkServer", "StartForkServer: command=%d\n", command);
    switch (command) {
      case SetPUTExecutionTimeoutCommand: {
        settings.timeout_us = RecvU64();
        LogInfo("ForkServer", "SetPUTExecutionTimeout: new timeout is %lu us\n",
                settings.timeout_us);
        break;
      }
      case ReadStdinCommand: {
        LogInfo("ForkServer", "ReadStdinCommand: activated\n");
        settings.read_stdin = true;
        break;
      }
      case SaveStdoutStderrCommand: {
        LogInfo("ForkServer", "SaveStdoutStderrCommand: activated\n");
        settings.save_stdout_stderr = true;
        break;
      }
      case ExecutePUTCommand: {
        bool i_am_put = ExecutePUT(&settings);
        if (i_am_put) {
          return;
        }
        LogSuccess("ForkServer", "ExecutePUTCommand: done\n");
        break;
      }
      default: {
        LogError("ForkServer", "Unknown command %d\n", command);
      }
    }

#ifdef SHOW_LOG
    fflush(log_file);
#endif
  }

  LogSuccess("ForkServer", "ForkServerEntryPoint: End\n");
}
