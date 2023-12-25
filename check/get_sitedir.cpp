/*
 * fuzzuf-cc
 * Copyright (C) 2021-2023 Ricerca Security
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
#include <iostream>
#include <string>
#include <Python.h>
int main() {
  Py_Initialize();
  auto site = PyImport_ImportModule( "site" );
  if ( !site ) return 1;
  auto getsitepackages = PyObject_GetAttrString( site, "getsitepackages" );
  if ( !getsitepackages ) return 1;
  auto getsitepackages_args = Py_BuildValue( "()" );
  if ( !getsitepackages_args ) return 1;
  auto path_list = PyObject_CallObject( getsitepackages, getsitepackages_args );
  if ( !path_list ) return 1;
  const auto list_size = PyList_Size( path_list );
  if( list_size == 0 ) return 1;
  for( size_t i = 0u; i != list_size; ++i ) {
    auto path = PyList_GetItem( path_list, i );
  if( !path ) return 1;
  auto encoded_path = PyUnicode_EncodeLocale( path, nullptr );
  if( !encoded_path ) return 1;
    std::cout << PyBytes_AS_STRING( encoded_path ) << ";" << std::flush;
  }
  Py_Finalize();
}
