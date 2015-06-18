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
set (DEFAULT_DB_PATH "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/db/minidlna" CACHE PATH "Default DB path")
set (DATA_PATH ${PROJECT_DATADIR})

option (TIVO_SUPPORT "enable TiVo support" OFF)
option (NETGEAR "enable generic NETGEAR device support" OFF)
option (READYNAS "enable NETGEAR ReadyNAS support" OFF)
option (ENABLE_VIDEO_THUMB "enable video thumbnail generation using libavcodec and libswscale" OFF)

if (READYNAS)
    set (NETGEAR ON)
    set (PNPX 5)
    set (TIVO_SUPPORT ON)
    set (OS_URL "http://www.readynas.com/")
endif()

if (NETGEAR)
    set (ROOTDEV_MANUFACTURERURL "http://www.netgear.com/")
    set (ROOTDEV_MANUFACTURER "NETGEAR")
    set (ROOTDEV_MODELNAME "Windows Media Connect compatible (ReadyDLNA)")
    set (ROOTDEV_MODELDESCRIPTION "ReadyDLNA")
endif()