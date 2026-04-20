cmake_minimum_required(VERSION 3.16)
project(GestionCollaborative)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network PrintSupport Charts)

add_executable(GestionCollaborative
    main.cpp
    utulisateur/loginwindow.h
    utulisateur/loginwindow.cpp
    utulisateur/userwindow.cpp
    utulisateur/userwindow.h
    connection/connection.h
    connection/connection.cpp
    matriel/matriele.cpp
    matriel/matriele.h
    matriel/qrcodegen.cpp
    matriel/qrcodegen.hpp
    matriel/matriele.ui
)

target_link_libraries(GestionCollaborative PRIVATE Qt6::Widgets Qt6::Sql Qt6::Network Qt6::PrintSupport Qt6::Charts)
set_target_properties(GestionCollaborative PROPERTIES AUTOMOC ON AUTOUIC ON)

target_include_directories(GestionCollaborative PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/utulisateur
    ${CMAKE_CURRENT_SOURCE_DIR}/connection
    ${CMAKE_CURRENT_SOURCE_DIR}/matriel
)
