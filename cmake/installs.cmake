include(GNUInstallDirs)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/qasterix)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#install(EXPORT QAsterixTargets
#  FILE QAsterixTargets.cmake
#  NAMESPACE QAsterix::
#  DESTINATION ${INSTALL_CONFIGDIR}
#)

#
# Install the whole stuff
#
install(TARGETS qasterix EXPORT QAsterixTargets
                         COMPONENT Library
                         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                         INCLUDES DESTINATION include
                         DESTINATION ${INSTALL_CONFIGDIR}
                         )

#install(FILES ${public_headers} COMPONENT Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qasterix)

include(CMakePackageConfigHelpers)

set(PROJECT_PREFIX bar-${QASTERIX_VERSION})

# Here we configure variables specifying the target directories.
set(INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})
set(INSTALL_CONFIG_DIR  ${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/cmake)
set(INSTALL_LIBRARY_DIR ${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/${CMAKE_BUILD_TYPE})
set(INSTALL_ARCHIVE_DIR ${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/${CMAKE_BUILD_TYPE}/static)
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_PREFIX}/bar)

# Define the configuration file locations.
set(PROJECT_CONFIG_VERSION_FILE "${PROJECT_BINARY_DIR}/bar-config-version.cmake")
set(PROJECT_CONFIG_FILE         "${PROJECT_BINARY_DIR}/bar-config.cmake")

write_basic_package_version_file(
    ${QASTERIX_BINARY_DIR}/QAsterixConfigVersion.cmake
    VERSION ${QASTERIX_VERSION_STRING}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  ${QASTERIX_MODULE_DIR}/config/config.cmake.in
  ${QASTERIX_BINARY_DIR}/QAsterixConfig.cmake
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qasterix
)

install(EXPORT QAsterixTargets
        FILE QAsterixTargets.cmake
        NAMESPACE QAsterix::
        DESTINATION lib/cmake/qasterix
        )

install(FILES
    ${QASTERIX_BINARY_DIR}/QAsterixConfig.cmake
    ${QASTERIX_BINARY_DIR}/QAsterixConfigVersion.cmak
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qasterix
    COMPONENT Devel
)

### Add support for an uninstall target
configure_file(
  "${QASTERIX_MODULE_DIR}/cmake-uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")



#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QAsterixLibrary.cmake DESTINATION ${INSTALL_CONFIGDIR})
