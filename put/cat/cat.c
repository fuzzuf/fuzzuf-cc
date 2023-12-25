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
// #include <stdlib.h>
// #include <sys/shm.h>
#include <string.h>
#include <unistd.h>

// extern void* __afl_area_ptr;

// // Unit test からだとAFLのビットマップ初期化ロジックが呼ばれずにSEGVする謎を回避する
// void Workaround() {
//     const char* shm_id = getenv("__AFL_SHM_ID");
//     if (shm_id) {
//         __afl_area_ptr = shmat(atoi(shm_id), 0, 0);
//     }
// }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv) {
#pragma GCC diagnostic pop
    // Workaround();

    char buf[128] = {0};
    if (read(0, buf, sizeof(buf)) < 0) {
        perror("Failed to read stdin:");
        return 1;
    }
    fprintf(stdout, "stdout: %s", buf);
    fprintf(stderr, "stderr: %s", buf);
    return 0;
}
