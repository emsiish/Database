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

include(CTest)


# Make Catch2 available
if(BUILD_TESTING)

    message(STATUS "Make Catch2 available...")

    if(EXISTS ${CMAKE_SOURCE_DIR}/lib/Catch2)

        add_subdirectory(${CMAKE_SOURCE_DIR}/lib/Catch2)

    else()
        include(FetchContent)
        FetchContent_Declare(
                Catch2
                GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
                GIT_TAG   v3.4.0
                # FIND_PACKAGE_ARGS
        )

        FetchContent_MakeAvailable(Catch2)

    endif()

    include(Catch)
endif()



# Tests
add_executable(test_runner)
target_link_libraries(
        test_runner
        PRIVATE
        db
        Catch2::Catch2WithMain
)
target_sources(
        test_runner
        PRIVATE
        "tests.cpp"
)
catch_discover_tests(test_runner)
