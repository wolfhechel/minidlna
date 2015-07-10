if (NOT FFmpeg_FIND_COMPONENTS)
    set (FFmpeg_FIND_COMPONENTS avcodec
                                avdevice
                                avfilter
                                avformat
                                avresample
                                avutil
                                postproc
                                swresample
                                swscale)

endif(NOT FFmpeg_FIND_COMPONENTS)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig QUIET)

macro (find_ffmpeg_component var component)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(_PKG_${var} lib${component})
    endif (PKG_CONFIG_FOUND)

    find_path(${var}_INCLUDE_DIRS
        NAMES lib${component}/${component}.h
        PATHS ${_PKG_${var}_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include /sw/include
        PATH_SUFFIXES ffmpeg libav
    )

    find_path(${var}_INCLUDE_DIRS
        NAMES ${component}.h
        PATHS ${_PKG_${var}_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include /sw/include
        PATH_SUFFIXES ffmpeg libav
    )

    find_library(${var}_LIBRARIES
        NAMES ${component}
        PATHS ${_PKG_${var}_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
    )

    mark_as_advanced(${var}_INCLUDE_DIRS ${var}_LIBRARIES)
endmacro(find_ffmpeg_component var component)

if (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)
  set(FFmpeg_FOUND TRUE)
else (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)
    foreach (component ${FFmpeg_FIND_COMPONENTS})
        find_ffmpeg_component(FFmpeg_${component} ${component})

        if (FFmpeg_${component}_LIBRARIES AND FFmpeg_${component}_INCLUDE_DIRS)
            list (APPEND FFmpeg_LIBRARIES "${FFmpeg_${component}_LIBRARIES}")
            list (APPEND FFmpeg_INCLUDE_DIRS "${FFmpeg_${component}_INCLUDE_DIRS}")
        else (FFmpeg_${component}_LIBRARIES AND FFmpeg_${component}_INCLUDE_DIRS)
            set (FFmpeg_MISSING_LIBRARIES "${FFmpeg_MISSING_LIBRARIES} lib${component}")
        endif (FFmpeg_${component}_LIBRARIES AND FFmpeg_${component}_INCLUDE_DIRS)

    endforeach (component ${FFmpeg_FIND_COMPONENTS})
endif (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)

if (NOT FFmpeg_MISSING_LIBRARIES)
    set (FFmpeg_FOUND TRUE)

    if (NOT FFmpeg_FIND_QUIETLY)
        message (STATUS "Found FFmpeg libraries ${FFmpeg_LIBRARIES}")
        message (STATUS "Found FFmpeg header search paths ${FFmpeg_INCLUDE_DIRS}")
    endif (NOT FFmpeg_FIND_QUIETLY)
else (NOT FFmpeg_MISSING_LIBRARIES)
    set (_missing_message "Could not find FFMpeg libraries ${FFmpeg_MISSING_LIBRARIES}")

    if (FFmpeg_FIND_QUIETLY)
        if (FFmpeg_FIND_REQUIRED)
            message (FATAL_ERROR "${_missing_message}")
        else (FFmpeg_FIND_REQUIRED)
            message (WARNING "${_missing_message}")
        endif (FFmpeg_FIND_REQUIRED)
    endif (FFmpeg_FIND_QUIETLY)
endif (NOT FFmpeg_MISSING_LIBRARIES)

# show the FFmpeg_INCLUDE_DIRS and FFmpeg_LIBRARIES variables only in the advanced view
mark_as_advanced(FFmpeg_INCLUDE_DIRS FFmpeg_LIBRARIES)