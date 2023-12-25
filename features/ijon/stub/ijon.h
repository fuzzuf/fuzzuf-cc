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
#pragma once
#ifndef _NO_IJON_IN_ASM
#if defined(__cplusplus)  
extern "C" {
#endif

#define IJON_BITS(x) (x)
#define IJON_INC(x) (x)
#define IJON_SET(x) (x)

#define IJON_CTX(x) (x)

#define IJON_MAX(x) (x)
#define IJON_MIN(x) (x)
#define IJON_CMP(x,y) (x == y)
#define IJON_DIST(x,y) (x == y)
#define IJON_STRDIST(x,y) (x == y)

#if defined(__cplusplus)  
}
#endif
#endif
