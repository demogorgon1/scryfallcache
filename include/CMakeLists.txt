cmake_minimum_required(VERSION 3.15)

target_include_directories(sfc PUBLIC  
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(sfc PRIVATE
	sfc/sfc.h
	sfc/sfc_app.h
	sfc/sfc_base.h
	sfc/sfc_cache.h
	sfc/sfc_card.h
	sfc/sfc_card_array.h
	sfc/sfc_curl.h
	sfc/sfc_debug.h
	sfc/sfc_query.h
)

install(DIRECTORY sfc DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})