cmake_minimum_required (VERSION 3.19)

project(sfc)

include(FetchContent)

# ------------------------------- curl -------------------------------
if(SFC_FORCE_OPENSSL_BACKEND OR SFC_FORCE_WINSSL_BACKEND OR SFC_FORCE_DARWINSSL_BACKEND)
	set(DETECT_SSL_BACKEND OFF CACHE INTERNAL "" FORCE)
else()
	set(DETECT_SSL_BACKEND ON CACHE INTERNAL "" FORCE)
endif()

if(SFC_FORCE_WINSSL_BACKEND AND (NOT WIN32))
	message(FATAL_ERROR "WinSSL is Windows only.")
endif()

if(DETECT_SSL_BACKEND)
	message(STATUS "Detecting SSL backend...")
	if(WIN32)
		message(STATUS "SSL auto detect: Using WinSSL.")
		set(SSL_BACKEND_USED "WinSSL")
	elseif(APPLE)
		message(STATUS "SSL auto detect: Using DarwinSSL.")
		set(SSL_BACKEND_USED "DarwinSSL")
	else()
		find_package(OpenSSL)
		if(OPENSSL_FOUND)
			message(STATUS "SSL auto detect: Using OpenSSL.")
			set(SSL_BACKEND_USED "OpenSSL")
		else()
			message(FATAL_ERROR "OpenSSL not found.")
		endif()
	endif()
else()
	if(SFC_FORCE_OPENSSL_BACKEND)
		find_package(OpenSSL)
		if(OPENSSL_FOUND)
			message(STATUS "Using OpenSSL.")
			set(SSL_BACKEND_USED "OpenSSL")
		else()
			message(FATAL_ERROR "OpenSSL not found.")
		endif()
	elseif(SFC_FORCE_WINSSL_BACKEND)
		message(STATUS "Using WinSSL.")
		set(SSL_BACKEND_USED "WinSSL")
	elseif(SFC_FORCE_DARWINSSL_BACKEND)
		message(STATUS "Using DarwinSSL.")
		set(SSL_BACKEND_USED "DarwinSSL")
	endif()
endif()

if(SFC_FORCE_USE_SYSTEM_CURL)
	find_package(CURL COMPONENTS HTTP HTTPS)
	if(CURL_FOUND)
		message(STATUS "Curl ${CURL_VERSION_STRING} found on this system.")
	else()
		find_package(CURL COMPONENTS HTTP)
		if(CURL_FOUND)
			message(FATAL_ERROR "Curl found on this system, but https isn't supported.")
		else()
			message(FATAL_ERROR "Curl not found on this system.")
		endif()
	endif()
else()
    message(STATUS "Configuring curl...")

    set(HTTP_ONLY ON CACHE INTERNAL "" FORCE)
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "" FORCE)
   	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
	set(BUILD_TESTING OFF)

	set(SSL_ENABLED ON CACHE INTERNAL "" FORCE)
	
	set(CURL_CA_PATH "auto" CACHE INTERNAL "")
	set(CURL_CA_BUNDLE "auto" CACHE INTERNAL "")
	
	if(SSL_BACKEND_USED STREQUAL "WinSSL")
		set(CURL_USE_SCHANNEL ON CACHE INTERNAL "" FORCE)
	endif()

	if(SSL_BACKEND_USED STREQUAL "OpenSSL")
		set(CURL_USE_OPENSSL ON CACHE INTERNAL "" FORCE)
	endif()

	if(SSL_BACKEND_USED STREQUAL "DarwinSSL")
		set(CURL_USE_SECTRANSP ON CACHE INTERNAL "" FORCE)
	endif()
	
	FetchContent_Declare(curl
	  GIT_REPOSITORY https://github.com/curl/curl.git
	  GIT_TAG        64db5c575d9c5536bd273a890f50777ad1ca7c13 # 7.82.0 
	)

	FetchContent_MakeAvailable(curl)

    set_property(TARGET libcurl PROPERTY FOLDER "external")	
endif()

# --------------------------------------------------------------------

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")



add_subdirectory(sfc)
add_subdirectory(test)
add_subdirectory(include)

#add_executable(sfc-test 
#test/test.c
#)
#
#target_link_libraries(sfc-test sfc CURL::libcurl)