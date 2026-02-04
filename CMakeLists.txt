cmake_minimum_required(VERSION 4.0)
project(proekt)

set(CMAKE_CXX_STANDARD 20)

add_library(db STATIC)

set_target_properties(db PROPERTIES OUTPUT_NAME "project")

target_sources(
        db
        PRIVATE
        Data.h
        Database.h
        Database.cpp
        Expression.h
        Index.h
        Index.cpp
        Parser.h
        Table.h
        Table.cpp
)

add_executable(application)

set_target_properties(application PROPERTIES OUTPUT_NAME "application")

target_link_libraries(
        application
        PRIVATE
        db
)

target_sources(
        application
        PRIVATE
        "main.cpp"
)
