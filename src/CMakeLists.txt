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
             upnpevents.c
             sql.c
             utils.c
             metadata.c
             scanner.c
             inotify.c
             playlist.c
             image_utils.c
             albumart.c
             log.c
             video_thumb.c
             containers.c
             tagutils/tagutils.c
             naturalsort.c)

set (HEADERS config.h
             libav.h
             scanner_sqlite.h
             sendfile.h
             minidlnapath.h
             minidlnatypes.h)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (FFMPEG_COMPONENTS avcodec avformat avutil)

if (ENABLE_VIDEO_THUMB)
    set (FFMPEG_COMPONENTS ${FFMPEG_COMPONENTS} swscale)
endif ()

find_package (FFmpeg COMPONENTS ${FFMPEG_COMPONENTS} REQUIRED)
include_directories (${FFmpeg_INCLUDE_DIRS})
list (APPEND minidlnad_LIBS ${FFmpeg_LIBRARIES})

find_package(JPEG REQUIRED)
include_directories (${JPEG_INCLUDE_DIRS})
list (APPEND minidlnad_LIBS ${JPEG_LIBRARIES})

find_package (EXIF REQUIRED)
include_directories (${EXIF_INCLUDE_DIRS})
list (APPEND minidlnad_LIBS ${EXIF_LIBRARIES})

find_package (ID3tag REQUIRED)
include_directories (${ID3tag_INCLUDE_DIRS})
list (APPEND minidlnad_LIBS ${ID3tag_LIBRARIES})

find_package (SQLite3 REQUIRED)
include_directories (${SQLite3_INCLUDE_DIRS})
list (APPEND minidlnad_LIBS ${SQLite3_LIBRARIES})

find_package (FLAC REQUIRED)
include_directories (${FLAC_INCLUDE_DIRS})
list (APPEND minidlnad_LIBS ${FLAC_LIBRARIES})

find_package (VorbisFile REQUIRED)
include_directories (${VorbisFile_INCLUDE_DIR})
list (APPEND minidlnad_LIBS ${VorbisFile_LIBRARY})

find_package (Iconv QUIET REQUIRED)
find_package (intl QUIET REQUIRED)

if (ICONV_FOUND AND INTL_FOUND)
    include_directories (${ICONV_INCLUDE_DIR})
    include_directories (${INTL_INCLUDE_DIR})

    list (APPEND minidlnad_LIBS ${ICONV_LIBRARY} ${INTL_LIBRARY})

    set (ENABLE_NLS TRUE)
endif ()

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
list (APPEND minidlnad_LIBS Threads::Threads)

get_property (CMAKE_REQUIRED_INCLUDES
              DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              PROPERTY INCLUDE_DIRECTORIES)

include (cmake/Includes/ConfigureChecks.cmake)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

add_executable(minidlnad ${SOURCES} ${HEADERS})

target_link_libraries (minidlnad ${minidlnad_LIBS})

# Install binary
install (TARGETS minidlnad
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})