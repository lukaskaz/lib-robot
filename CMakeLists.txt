cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(robot)

include(cmake/dependencies.cmake)
include(cmake/flags.cmake)

option(RUN_TESTS "Creates and runs unit tests for the project" OFF)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include_directories(inc)
file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})
add_dependencies(${PROJECT_NAME} libmenu)
add_dependencies(${PROJECT_NAME} libtts)
add_dependencies(${PROJECT_NAME} libhttp)
add_dependencies(${PROJECT_NAME} libshellcmd)
add_dependencies(${PROJECT_NAME} liblogger)
add_dependencies(${PROJECT_NAME} librandom)

target_link_libraries(${PROJECT_NAME}
    menu
    tts
    http
    shellcmd
    logger
    ${Boost_LIBRARIES}
)

