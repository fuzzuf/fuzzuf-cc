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
void do_nothing() {
  // bitmap[0] should be here
  // NOTE: the order of bitmap is not guaranteed and depends on the
  // implementation
}

int main() {
  // we want to observe bitmap[0] when it is updated 0x100 times, but bitmap[0]
  // is initialized to 0x1 in AFL, so we call do_nothing() 0xff times
  // ref:
  // https://github.com/mboehme/aflfast/blob/master/llvm_mode/afl-llvm-rt.o.c#L86
  for (int i = 0; i < 0xff; i++) {
    do_nothing();
  }
}
