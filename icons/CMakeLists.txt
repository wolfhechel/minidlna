# Install shared data
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         DESTINATION ${PROJECT_DATADIR}
         PATTERN "CMakeLists.txt" EXCLUDE)