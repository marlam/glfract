# Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 3.10)
set(CMAKE_AUTOMOC ON)

project(qv)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt6 6.2.0 COMPONENTS OpenGLWidgets)

qt6_add_resources(GUI_RESOURCES gui.qrc)
add_executable(glfract 
	gui.hpp gui.cpp
        glwidget.hpp glwidget.cpp
	state.hpp state.cpp
	${GUI_RESOURCES})
target_link_libraries(glfract -lquadmath Qt6::OpenGLWidgets)
install(TARGETS glfract RUNTIME DESTINATION bin)
