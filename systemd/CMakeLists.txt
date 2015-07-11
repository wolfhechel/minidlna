configure_file (minidlna.service.in
                systemd/system/minidlna.service)

configure_file (minidlna.sysusers.in
                sysusers.d/minidlna.conf)

configure_file (minidlna.tmpfiles.in
                tmpfiles.d/minidlna.conf)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/tmpfiles.d/minidlna.conf
         DESTINATION ${SYSTEMD_LIB_DIR}/tmpfiles.d)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/sysusers.d/minidlna.conf
         DESTINATION ${SYSTEMD_LIB_DIR}/sysusers.d)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/systemd/system/minidlna.service
         DESTINATION ${SYSTEMD_LIB_DIR}/systemd/system)