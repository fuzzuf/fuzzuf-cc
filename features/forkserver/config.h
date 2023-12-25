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
// #define SHOW_LOG y

// #define FORKSRV_FD_READ 198
// #define FORKSRV_FD_WRITE 199

// afl-gcc を使い回す都合で、本家とバッチングしない値を使う
#define FORKSRV_FD_READ 196
#define FORKSRV_FD_WRITE 197

// #define FORKSRV_FD_READ 0
// #define FORKSRV_FD_WRITE 1

#define MAX_FILE_PATH_LENGTH 256
const char PREDEFINED_STDXXX_FILE_PATH[] = "/dev/shm/fuzzuf-cc.forkserver.executor_id-%lu.%s";

typedef enum _ForkServerAPI {
    InvalidCommand = 0,
    SetPUTExecutionTimeoutCommand,
    ReadStdinCommand,
    SaveStdoutStderrCommand,
    ExecutePUTCommand,
} ForkServerAPI;

typedef enum {
    None = 0,
    DaemonAlreadyExitError,
    DaemonBusyError,
    SpawnPUTError,
    UnknownPUTStateError,
    NoResponseError,
} ExecutePUTError;

// TODO: Use protocol buffer
typedef struct {
    ExecutePUTError error;
    int32_t exit_code;
    int32_t signal_number;
} ExecutePUTAPIResponse;
