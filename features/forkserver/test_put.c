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
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <string.h>
#include <unistd.h>

#define STREQ(a, b) (strncmp(a, b, sizeof(a)) == 0)

int main(int argc, char** argv) {
    if (argc > 1) {
        if (STREQ("sleep", argv[1])) {
            printf("[*] [PUT] Sleep...\n");
            sleep(10);
        } else if (STREQ("stdin", argv[1])) {
            printf("[*] [PUT] Read from stdin: ");
            char buf[128] = "";
            while (read(0, buf, sizeof(buf)) > 0) {
                printf("%s", buf);
                memset(buf, 0, sizeof(buf) );
            }
        } else {
            fprintf(stdout, "[*] [PUT:stdout] Message \"%s\"\n", argv[1]);
            fprintf(stderr, "[*] [PUT:stderr] Message \"%s\"\n", argv[1]);
        }
        return 115; // Just check if forkserver can obtain correct return value
    }
    if (argc > 2) {
        raise(SIGABRT);
    }

    if (argc == 1) {
        return 114; // Just check if forkserver can obtain correct return value
    }
}
