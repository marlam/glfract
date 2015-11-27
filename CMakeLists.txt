# Copyright (C) 2015  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1y -Wall -Wextra")
endif()

find_package(Qt5Widgets REQUIRED)

qt5_add_resources(GUI_RESOURCES gui.qrc)
add_executable(glfract 
	gui.hpp gui.cpp
        glwidget.hpp glwidget.cpp
	state.hpp state.cpp
	${GUI_RESOURCES})
target_link_libraries(glfract -lquadmath Qt5::Widgets)
install(TARGETS glfract RUNTIME DESTINATION bin)
