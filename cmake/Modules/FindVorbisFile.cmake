# - Try to find VorbisFile
# Once done this will define
#  VorbisFile_FOUND - System has VorbisFile
#  VorbisFile_INCLUDE_DIRS - The VorbisFile include directories
#  VorbisFile_LIBRARIES - The libraries needed to use VorbisFile
#  VorbisFile_DEFINITIONS - Compiler switches required for using VorbisFile

find_package(PkgConfig)
pkg_check_modules(PC_VorbisFile vorbisfile)
set(VorbisFile_DEFINITIONS ${PC_VorbisFile_CFLAGS_OTHER})

find_path(VorbisFile_INCLUDE_DIR vorbis/vorbisfile.h
          HINTS ${PC_VorbisFile_INCLUDEDIR} ${PC_VorbisFile_INCLUDE_DIRS})

find_library(VorbisFile_LIBRARY NAMES vorbis
             HINTS ${PC_VorbisFile_LIBDIR} ${PC_VorbisFile_LIBRARY_DIRS} )

set(VorbisFile_LIBRARIES ${VorbisFile_LIBRARY})
set(VorbisFile_INCLUDE_DIRS ${VorbisFile_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(VorbisFile DEFAULT_MSG
                                  VorbisFile_LIBRARY VorbisFile_INCLUDE_DIR)

mark_as_advanced(VorbisFile_INCLUDE_DIR VorbisFile_INCLUDE_DIRS VorbisFile_LIBRARY VorbisFile_LIBRARIES)