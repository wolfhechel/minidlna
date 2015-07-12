cmake_minimum_required (VERSION 3.1)

project (MiniDLNA)

string (TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)

set (MiniDLNA_VERSION_MAJOR 1)
set (MiniDLNA_VERSION_MINOR 1)
set (MiniDLNA_VERSION_PATCH 4)
set (MiniDLNA_VERSION "${MiniDLNA_VERSION_MAJOR}.${MiniDLNA_VERSION_MINOR}.${MiniDLNA_VERSION_PATCH}")
set (MiniDLNA_PACKAGE_NAME ${PACKAGE_NAME})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

message (STATUS ${CMAKE_MODULE_PATH})

include (GNUInstallDirs)

set (PROJECT_DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/${PACKAGE_NAME}"
     CACHE PATH "Projects data directory [DATADIR]/${PACKAGE_NAME}]")

include_directories (${CMAKE_SOURCE_DIR})
include_directories (${CMAKE_BINARY_DIR})

set (OS_NAME "${CMAKE_SYSTEM_NAME}" CACHE STRING "OS Name")
set (OS_VERSION "${CMAKE_SYSTEM_VERSION}" CACHE STRING "OS Version")
set (OS_URL "http://www.netgear.com" CACHE STRING "OS URL")

set (ROOTDEV_MANUFACTURER "Justin Maggard")
set (ROOTDEV_MANUFACTURERURL "http://www.netgear.com/")
set (ROOTDEV_MODELNAME "Windows Media Connect compatible (MiniDLNA)")
set (ROOTDEV_MODELDESCRIPTION "MiniDLNA on ${OS_NAME}")
set (ROOTDEV_MODELURL "${OS_URL}")

set (DEFAULT_LOG_PATH "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/log" CACHE PATH "Default log path")
set (DEFAULT_RUN_PATH "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/run" CACHE PATH "Default run path")
set (DEFAULT_DB_PATH "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/cache/minidlna" CACHE PATH "Default DB path")
set (DEFAULT_CONF_PATH "${CMAKE_INSTALL_FULL_SYSCONFDIR}/minidlna.conf" CACHE PATH "Default configuration path")

set (DATA_PATH ${PROJECT_DATADIR})

set (DEFAULT_USER "minidlna" CACHE STRING "Default user to run as")
set (DEFAULT_GROUP "minidlna" CACHE STRING "Default group to run as")

set (SYSTEMD_LIB_DIR "${CMAKE_INSTALL_FULL_LIBDIR}" CACHE PATH "systemd library path")

option (ENABLE_VIDEO_THUMB "enable video thumbnail generation using libavcodec and libswscale" ON)

option (ENABLE_SYSTEMD "enable systemd support" ON)

if (LINUX)
    set (ENABLE_SYSTEMD ON)
endif ()

add_subdirectory (po)
add_subdirectory (src)
add_subdirectory (man)
add_subdirectory (icons)

if (ENABLE_SYSTEMD)
    add_subdirectory (systemd)
endif ()

configure_file(minidlna.conf.in minidlna.conf)

# Install configuration
install (FILES ${CMAKE_BINARY_DIR}/minidlna.conf
         DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})