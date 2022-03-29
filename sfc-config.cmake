find_path(SFC_INCLUDE_DIR
    NAMES sfc.h)

find_library(SFC_LIBRARY
	NAMES sfc
    HINTS ${SFC_LIBRARY_ROOT})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SFC REQUIRED_VARS SFC_LIBRARY SFC_INCLUDE_DIR)

if(SFC_FOUND)
    set(SFC_LIBRARIES ${SFC_LIBRARY})
    set(SFC_INCLUDE_DIRS ${SFC_INCLUDE_DIR})
endif()