cmake_minimum_required (VERSION 3.19)

if (NOT MSVC)
	# One test case includes a very long string
	add_compile_options(-Wno-overlength-strings)
endif()

add_executable(sfc-test
	test.c)
	
target_link_libraries(sfc-test sfc::sfc)

add_test(sfc-test sfc-test)