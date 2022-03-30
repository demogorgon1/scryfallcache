cmake_minimum_required (VERSION 3.19)

add_executable(sfc-test
	test.c)
	
target_link_libraries(sfc-test sfc::sfc)

add_test(sfc-test sfc-test)