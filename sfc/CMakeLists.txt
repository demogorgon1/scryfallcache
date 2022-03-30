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

add_library(sfc STATIC
	sfc_app.c
	sfc_buffer.c
	sfc_buffer.h
	sfc_cache.c
	sfc_card.c
	sfc_card_array.c
	sfc_card_map_uint32.c
	sfc_card_map_uint32.h
	sfc_card_set.c
	sfc_card_set.h
	sfc_curl.c
	sfc_debug.c
	sfc_deserializer.c
	sfc_deserializer.h
	sfc_file.c
	sfc_file.h
	sfc_leaky_bucket.c
	sfc_leaky_bucket.h
	sfc_query.c
	sfc_serializer.c
	sfc_serializer.h
	sfc_string_set.c
	sfc_string_set.h
)

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