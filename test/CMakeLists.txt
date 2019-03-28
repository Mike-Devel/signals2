# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt
#
# NOTE: CMake support for Boost.Signals2 is currently experimental at best
#       and the interface is likely to change in the future

if(MSVC)
	STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	add_definitions( /D_SILENCE_CXX17_SHARED_PTR_UNIQUE_DEPRECATION_WARNING /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS )
	add_compile_options( /permissive- )
else()
	add_compile_options( -Wall )
endif()

file( GLOB test_src_files *.cpp )

list(REMOVE_ITEM test_src_files ${CMAKE_CURRENT_LIST_DIR}/invocation_benchmark.cpp)
# list(REMOVE_ITEM test_src_files ${CMAKE_CURRENT_LIST_DIR}/signals_vs_signals2_benchmark.cpp)

find_package(Threads REQUIRED)

 foreach( file IN LISTS test_src_files )

	get_filename_component( test_name ${file} NAME_WE )
	set( executable_name test_boost_signals2_${test_name} )
	add_executable( ${executable_name} ${file} )
	target_link_libraries( ${executable_name}
		PUBLIC
			Boost::signals2
			Boost::thread
			Boost::timer
			Boost::function
			Threads::Threads
	)
	add_test( NAME ${executable_name} COMMAND ${executable_name} )

endforeach()

# checking that headerfiles compiles on its own (has all necessary includes)
file( GLOB header_files ../include/boost/signals2/*.hpp )

foreach( file IN LISTS header_files )

	get_filename_component( header_name ${file} NAME )

	set( executable_name test_boost_signals2_compile_${header_name} )
	set( test_file_name ${CMAKE_CURRENT_BINARY_DIR}/test_boost_signals2_compile_${header_name}.cpp )
	file(GENERATE OUTPUT ${test_file_name}
		CONTENT
		"#include <boost/signals2/${header_name}>
		int main(){}
		"
	)

	add_executable( ${executable_name} ${test_file_name} )
	target_link_libraries( ${executable_name}
		PUBLIC
			Boost::signals2
	)

endforeach()

# checking that headerfiles compiles on its own (has all necessary includes)
file( GLOB header_files ../include/boost/signals2/detail/*.hpp )

foreach( file IN LISTS header_files )

	get_filename_component( header_name ${file} NAME )

	set( executable_name test_boost_signals2_compile_detail_${header_name} )
	set( test_file_name ${CMAKE_CURRENT_BINARY_DIR}/detail/test_boost_signals2_compile_${header_name}.cpp )
	file(GENERATE OUTPUT ${test_file_name}
		CONTENT
		"#include <boost/signals2/detail/${header_name}>
		int main(){}
		"
	)

	add_executable( ${executable_name} ${test_file_name} )
	target_link_libraries( ${executable_name}
		PUBLIC
			Boost::signals2
	)

endforeach()
