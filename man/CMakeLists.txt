# Install manual pages
# TODO: Compress manual pages
foreach (manpage RANGE 0 8)
    FILE(GLOB manfiles "${CMAKE_CURRENT_SOURCE_DIR}/*.${manpage}")

    install (FILES ${manfiles} DESTINATION ${CMAKE_INSTALL_MANDIR}/man${manpage})
endforeach ()