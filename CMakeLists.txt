cmake_minimum_required (VERSION 2.8)

project (MiniDLNA)

string (TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)

set (MiniDLNA_VERSION_MAJOR 1)
set (MiniDLNA_VERSION_MINOR 1)
set (MiniDLNA_VERSION_PATCH 4)
set (MiniDLNA_VERSION "${MiniDLNA_VERSION_MAJOR}.${MiniDLNA_VERSION_MINOR}.${MiniDLNA_VERSION_PATCH}")
set (MiniDLNA_PACKAGE_NAME ${PACKAGE_NAME})

set (SOURCES minidlna.c
             upnphttp.c
             upnpdescgen.c
             upnpsoap.c
             upnpreplyparse.c
             minixml.c
             clients.c
             getifaddr.c
             process.c
             upnpglobalvars.c
             options.c
             minissdp.c
             uuid.c
             upnpevents.c
             sql.c
             utils.c
             metadata.c
             scanner.c
             inotify.c
             tivo_utils.c
             tivo_beacon.c
             tivo_commands.c
             playlist.c
             image_utils.c
             albumart.c
             log.c
             video_thumb.c
             containers.c
             tagutils/tagutils.c
             naturalsort.c)

set (HEADERS codelength.h
             config.h
             libav.h
             scanner_sqlite.h
             sendfile.h
             minidlnapath.h
             minidlnatypes.h)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include (GNUInstallDirs)

set (PROJECT_DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/${PACKAGE_NAME}"
     CACHE PATH "Projects data directory [DATADIR]/${PACKAGE_NAME}]")

include_directories (${CMAKE_SOURCE_DIR})
include_directories (${CMAKE_BINARY_DIR})

include (cmake/Includes/DefineOptions.cmake)

set (FFMPEG_COMPONENTS avcodec avformat avutil)

if (ENABLE_VIDEO_THUMB)
    set (FFMPEG_COMPONENTS ${FFMPEG_COMPONENTS} swscale)
endif ()

find_package (FFmpeg COMPONENTS ${FFMPEG_COMPONENTS} QUIET REQUIRED)

if (FFmpeg_FOUND)
    include_directories (${FFmpeg_INCLUDE_DIRS})

    set (minidlnad_LIBS ${minidlnad_LIBS}
                        ${FFmpeg_LIBRARIES})
endif ()

find_package(JPEG QUIET REQUIRED)

if (JPEG_FOUND)
    include_directories (${JPEG_INCLUDE_DIRS})

    set (minidlnad_LIBS ${minidlnad_LIBS}
                        ${JPEG_LIBRARIES})
endif ()

find_package (EXIF QUIET REQUIRED)

if (EXIF_FOUND)
    include_directories (${EXIF_INCLUDE_DIRS})

        set (minidlnad_LIBS ${minidlnad_LIBS}
                            ${EXIF_LIBRARIES})
endif ()

find_package (ID3tag QUIET REQUIRED)

if (ID3tag_FOUND)
    include_directories (${ID3tag_INCLUDE_DIRS})

    set (minidlnad_LIBS ${minidlnad_LIBS}
                        ${ID3tag_LIBRARIES})
endif ()

find_package (SQLite3 QUIET REQUIRED)

if (SQLite3_FOUND)
    include_directories (${SQLite3_INCLUDE_DIRS})

        set (minidlnad_LIBS ${minidlnad_LIBS}
                            ${SQLite3_LIBRARIES})
endif ()

find_package (FLAC QUIET REQUIRED)

if (FLAC_FOUND)
    include_directories (${FLAC_INCLUDE_DIRS})

    set (minidlnad_LIBS ${minidlnad_LIBS}
                        ${FLAC_LIBRARIES})
endif ()

find_package (VorbisFile QUIET REQUIRED)

if (VORBISFILE_FOUND)
    include_directories (${VorbisFile_INCLUDE_DIR})

    set (minidlnad_LIBS ${minidlnad_LIBS}
                        ${VorbisFile_LIBRARY})
endif ()

find_package (Iconv QUIET REQUIRED)
find_package (intl QUIET REQUIRED)

if (ICONV_FOUND AND INTL_FOUND)
    include_directories (${ICONV_INCLUDE_DIR})
    include_directories (${INTL_INCLUDE_DIR})

    set (minidlnad_LIBS ${minidlnad_LIBS}
                        ${ICONV_LIBRARY}
                        ${INTL_LIBRARY})

    set (ENABLE_NLS TRUE)
endif ()

add_subdirectory (po)

get_property (CMAKE_REQUIRED_INCLUDES
              DIRECTORY ${CMAKE_SOURCE_DIR}
              PROPERTY INCLUDE_DIRECTORIES)

include (cmake/Includes/ConfigureChecks.cmake)

configure_file (
    "${CMAKE_SOURCE_DIR}/config.h.in"
    "${CMAKE_BINARY_DIR}/config.h"
)

add_executable(minidlnad ${SOURCES} ${HEADERS})

add_custom_command(TARGET minidlnad PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/icons "${CMAKE_BINARY_DIR}/${PROJECT_DATADIR}")

target_link_libraries (minidlnad ${minidlnad_LIBS})

# Install binary
install (TARGETS minidlnad
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install shared data
install (DIRECTORY icons
         DESTINATION ${PROJECT_DATADIR})

# Install configuration
install (FILES minidlna.conf
         DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

# Install manual pages
# TODO: Compress manual pages
foreach (manpage RANGE 0 8)
    FILE(GLOB manfiles "${CMAKE_CURRENT_SOURCE_DIR}/*.${manpage}")

    install (FILES ${manfiles} DESTINATION ${CMAKE_INSTALL_MANDIR}/man${manpage})
endforeach ()