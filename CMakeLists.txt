# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt
#
# NOTE: CMake support for Boost.Signals2 is currently experimental at best
#       and the interface is likely to change in the future

cmake_minimum_required( VERSION 3.5 )
project( BoostSignals2 LANGUAGES CXX)
option( BOOST_SIGNALS2_INCLUDE_TESTS "Add boost signals2 tests" OFF )

add_library( boost_signals2 INTERFACE )
add_library( Boost::signals2 ALIAS boost_signals2 )

target_include_directories( boost_signals2 INTERFACE include )

find_package(Threads REQUIRED)

target_link_libraries( boost_signals2
    INTERFACE
        Boost::assert
		Boost::bind
        Boost::config
        Boost::core
        Boost::function
        Boost::iterator
        Boost::mpl
        Boost::optional
        Boost::parameter
        Boost::throw_exception
        Boost::type_traits
        Boost::variant
		Threads::Threads
)

target_compile_features(boost_signals2 INTERFACE cxx_std_17 )

if( BOOST_SIGNALS2_INCLUDE_TESTS )
    enable_testing()
    add_subdirectory( test )
endif()

