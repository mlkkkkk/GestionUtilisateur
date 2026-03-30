cmake_minimum_required(VERSION 3.16)
project(GestionCollaborative)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(GestionCollaborative
    main.cpp
    loginwindow.h
    loginwindow.cpp
    userwindow.cpp
    userwindow.h
    connection.h
    connection.cpp
)

target_link_libraries(GestionCollaborative PRIVATE Qt6::Widgets Qt6::Sql)
set_target_properties(GestionCollaborative PROPERTIES AUTOMOC ON)
