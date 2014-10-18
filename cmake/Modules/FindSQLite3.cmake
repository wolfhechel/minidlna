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

if(SQLite3_INCLUDE_DIRS AND SQLite3_LIBRARIES)
   set(SQLite3_FOUND TRUE)

else(SQLite3_INCLUDE_DIRS AND SQLite3_LIBRARIES)

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

  if(SQLite3_INCLUDE_DIRS AND SQLite3_LIBRARIES)
    set(SQLite3_FOUND TRUE)
    message(STATUS "Found SQLite3: ${SQLite3_INCLUDE_DIRS}, ${SQLite3_LIBRARIES}")
  else(SQLite3_INCLUDE_DIRS AND SQLite3_LIBRARIES)
    set(SQLite3_FOUND FALSE)
    message(STATUS "SQLite3 not found.")
  endif(SQLite3_INCLUDE_DIRS AND SQLite3_LIBRARIES)

  mark_as_advanced(SQLite3_INCLUDE_DIRS SQLite3_LIBRARIES)

endif(SQLite3_INCLUDE_DIRS AND SQLite3_LIBRARIES)
