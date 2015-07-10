# - Find SQLite3
# Find the SQLite includes and library
# This module defines
#  SQLite3_INCLUDE_DIRS, where to find mysql.h
#  SQLite3_LIBRARIES, the libraries needed to use MySQL.
#  SQLite3_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig)
pkg_check_modules(SQLite3 sqlite3)

if (NOT SQLite3_INCLUDE_DIRS)
  set (SQLite3_INCLUDE_DIRS ${SQLite3_INCLUDEDIR})
endif (NOT SQLite3_INCLUDE_DIRS)

if (NOT SQLite3_FOUND)
  find_path(SQLite3_INCLUDE_DIRS SQLite3.h
      /usr/include
      /usr/local/include
      $ENV{ProgramFiles}/SQLite/include
      $ENV{SystemDrive}/SQLite/include
      $ENV{ProgramFiles}/SQLite
      $ENV{SystemDrive}/SQLite
      $ENV{ProgramFiles}/SQLite3/include
      $ENV{SystemDrive}/SQLite3/include
      $ENV{ProgramFiles}/SQLite3
      $ENV{SystemDrive}/SQLite3
      )

  find_library(SQLite3_LIBRARIES NAMES SQLite3
      PATHS
      /usr/lib
      /usr/local/lib
      $ENV{ProgramFiles}/SQLite/lib
      $ENV{SystemDrive}/SQLite/lib
      $ENV{ProgramFiles}/SQLite
      $ENV{SystemDrive}/SQLite
      $ENV{ProgramFiles}/SQLite3/lib
      $ENV{SystemDrive}/SQLite3/lib
      $ENV{ProgramFiles}/SQLite3
      $ENV{SystemDrive}/SQLite3
      )

endif(NOT SQLite3_FOUND)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SQLite3 DEFAULT_MSG
                                  SQLite3_LIBRARIES SQLite3_INCLUDE_DIRS)

mark_as_advanced(SQLite3_INCLUDE_DIRS SQLite3_LIBRARIES)
