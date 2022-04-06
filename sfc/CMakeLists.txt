cmake_minimum_required(VERSION 3.15)

add_compile_definitions(
	_CRT_SECURE_NO_WARNINGS
)

if(SFC_CURL)
	include_directories(
		include
		${curl_SOURCE_DIR}
	)
else()
	include_directories(
		include
	)
endif()

file(GLOB C_FILES "*.c")
file(GLOB H_FILES "*.h")

if(NOT SFC_CURL)
	list(FILTER C_FILES EXCLUDE REGEX ".*sfc_curl\\.c$")
endif()

add_library(sfc STATIC ${C_FILES} ${H_FILES})

add_library(sfc::sfc ALIAS sfc)

if(SFC_CURL)
	target_link_libraries(sfc PUBLIC CURL::libcurl)
endif()

if(SFC_FORCE_USE_SYSTEM_CURL AND SFC_CURL)
    install(TARGETS sfc
		EXPORT sfcTargets
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

    # Include CMake helpers for package config files
    # Follow this installation guideline: https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/sfc/sfcConfigVersion.cmake"
        VERSION ${${PROJECT_NAME}_VERSION}
        COMPATIBILITY ExactVersion)

    configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/sfcConfig.cmake.in
        "${PROJECT_BINARY_DIR}/sfc/sfcConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sfc)

    install(EXPORT sfcTargets
        FILE sfcTargets.cmake
        NAMESPACE sfc::
	    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sfc)

    install(FILES ${PROJECT_BINARY_DIR}/sfc/sfcConfig.cmake
        ${PROJECT_BINARY_DIR}/sfc/sfcConfigVersion.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sfc)     
else()
    install(TARGETS sfc
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()