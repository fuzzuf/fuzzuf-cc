/*
 * fuzzuf_cc
 * Copyright (C) 2022 Ricerca Security
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
/**
 * @file remove_cvr.hpp
 * @author Ricerca Security <fuzzuf-dev@ricsec.co.jp>
 */
#ifndef FUZZUF_CC_INCLUDE_UTILS_REMOVE_CVR_HPP
#define FUZZUF_CC_INCLUDE_UTILS_REMOVE_CVR_HPP
#include <type_traits>
namespace fuzzuf_cc::utils::type_traits {
/**
 * Remove reference from T, then remove const and volatile
 * This is compatible to C++20 std::remove_cvref_t
 *
 * example:
 * RemoveCvrT< const int& >
 * This is equivalent to int
 * RemoveCvrT< int&& >
 * This is also equivalent to int
 * RemoveCvrT< int >
 * This is also equivalent to int
 * RemoveCvrT< const int* >
 * This is equivalent to int*
 */
template <typename T>
#if __cplusplus >= 202002L
using RemoveCvrT = std::remove_cvref_t<T>;
#else
using RemoveCvrT = std::remove_cv_t<std::remove_reference_t<T> >;
#endif
}  // namespace fuzzuf_cc::utils::type_traits
#endif
