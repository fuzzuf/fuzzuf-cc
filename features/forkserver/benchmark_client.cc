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
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <cassert>

#include <string>
#include <chrono>
#include <iostream>

#include "config.h"

int forksrv_write_fd = -1;
int forksrv_read_fd = -1;

void AttachToServer(uint64_t executor_id) {
    if (write(forksrv_write_fd, &executor_id, sizeof(executor_id)) < (int)sizeof(executor_id)) {
        perror("[!] [Bench] Failed to attach to server");
        exit(1);
    }
    fprintf(stderr, "[*] [Bench] Requested server to attach: executor_id=%lu\n", executor_id);
}

pid_t WaitForkServerStart() {
    pid_t forksrv_pid = 0;
    int nbytes = read(forksrv_read_fd, &forksrv_pid, sizeof(forksrv_pid));
    if (nbytes < (int)sizeof(forksrv_pid)) {
        fprintf(stderr, "\tRequested to read %lu bytes, but read %d bytes\n", sizeof(forksrv_pid), nbytes);
        perror("[!] [Bench] Failed to wait for server start");
        exit(1);
    }
    fprintf(stderr, "[*] [Bench] Forkserver started: pid=%d\n", forksrv_pid);
    return forksrv_pid;
}

pid_t SetupForkserver(int /*pargc*/, char *const pargv[]) {
    int par2chld[2], chld2par[2];

    if (pipe(par2chld) || pipe(chld2par)) {
        perror("pipe() failed");
        exit(1);
    }

    pid_t forksrv_pid = fork();
    if (forksrv_pid < 0) {
        perror("fork() failed");
        exit(1);
    }

    if (forksrv_pid == 0) {
        // In PUT process
        if (dup2(par2chld[0], FORKSRV_FD_READ) < 0) {
            perror("dup2() failed");
            exit(1);
        };
        if (dup2(chld2par[1], FORKSRV_FD_WRITE) < 0) {
            perror("dup2() failed");
            exit(1);
        }

        close(par2chld[0]);
        close(par2chld[1]);
        close(chld2par[0]);
        close(chld2par[1]);

        fprintf(stderr, "[*] [Bench] pargv[0]=\"%s\": pid=%d\n", pargv[0], getpid());

        execv(pargv[0], pargv);
        exit(0);
    }

    close(par2chld[0]);
    close(chld2par[1]);

    forksrv_write_fd = par2chld[1];
    forksrv_read_fd = chld2par[0];

    AttachToServer(1);
    assert(WaitForkServerStart() == forksrv_pid);

    return forksrv_pid;
}

int Send(void *buf, size_t size, const char *name) {
    int nbytes = write(forksrv_write_fd, buf, size);
    if (nbytes < 0) {
        std::string message("[!] [Bench] Failed to request ");
        message += std::string(name);
        perror(message.c_str());
        exit(1);
    }
    return nbytes;
}

int SendCommand(ForkServerAPI command, const char *name) {
    return Send(&command, sizeof(command), name);
}

void ExecutePUT(bool show_response) {
    SendCommand(ExecutePUTCommand, "ExecutePUT");

    ExecutePUTAPIResponse response;
    response.error = NoResponseError; // read() might terminated by accidently
    if (read(forksrv_read_fd, &response, sizeof(ExecutePUTAPIResponse)) < 0) {
        perror("[!] [Bench] Failed to get exit status from PUT");
        exit(1);
    }
    if (show_response) {
        printf("[*] Response { error=%d, exit_code=%d }\n", response.error, response.exit_code);
    }
    assert(response.error == None);
}

void SetPUTExecutionTimeout(uint64_t timeout_us) {
    SendCommand(SetPUTExecutionTimeoutCommand, "SetPUTExecutionTimeout");
    Send(&timeout_us, sizeof(timeout_us), "SetPUTExecutionTimeout_Value");
}

int GetN(){
    char *N = getenv("N");
    if (N == NULL) {
        return 100000;
    } else {
        return atoi(N);
    }
}

std::string GetTestCase() {
    char *test_case = getenv("TEST");
    if (test_case != NULL) {
        return std::string(test_case);
    } else {
        return "";
    }
}

void SetupTest(std::string test_case) {
    if (test_case == "stdin") {
        fprintf(stderr, "[*] [Bench] Testing stdin\n");
        SendCommand(ReadStdinCommand, "ReadStdinCommand");
        return;
    }
    if (test_case == "stdout") {
        fprintf(stderr, "[*] [Bench] Testing stdout\n");
        SendCommand(SaveStdoutStderrCommand, "SaveStdoutStderrCommand");
        return;
    }
    fprintf(stderr, "[!] [Bench] Did nothing for case %s\n", test_case.c_str());
}

int main(int argc, char *const argv[]) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
    pid_t forksrv_pid = SetupForkserver(argc - 1, &argv[1]);
#pragma GCC diagnostic pop
    auto test_case = GetTestCase();
    if (test_case.length() > 0) {
        SetupTest(test_case);
    }

    SetPUTExecutionTimeout(40 * 1000);

    const uint64_t N = GetN();
    fprintf(stderr, "[*] [Bench] N = %ld\n", N);
    fflush(stderr);

    bool show_response = N <= 100;

    // Warm up: 最初は遅い
    if (N > 10) {
        for (std::uint64_t i = 0u; i < N / 10u; i += 1u) {
            ExecutePUT(show_response);
        }
        std::cerr << "[*] [Bench] Warm up done" << std::endl;
    }

    std::chrono::steady_clock::time_point checkpoint = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (std::uint64_t i = 0u; i < N; i += 1u) {
        if (N > 10u && i > 0u && i % (N / 10u) == 0u) {
            checkpoint = std::chrono::steady_clock::now();
            std::cerr << "Elapsed time at i=" << i << ": " << std::chrono::duration_cast<std::chrono::microseconds>(checkpoint - begin).count() << " [µs]" << std::endl;
        }

        ExecutePUT(show_response);
    }
    std::cerr << "----" << std::endl;

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    auto last_10p_elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - checkpoint).count();
    std::cerr << "Number of iteration = " << N << " [times]" << std::endl;
    std::cerr << "Total iteration time = " << elapsed_time << " [µs]" << std::endl;
    std::cerr << "Iteration time = " << elapsed_time / N << " [µs/iter]" << std::endl;
    std::cerr << "Total last 10\% iteration time = " << last_10p_elapsed_time << " [µs]" << std::endl;
    std::cerr << "Last 10\% Iteration time = " << last_10p_elapsed_time * 10 / N << " [µs/iter]" << std::endl;

    return 0;
}
