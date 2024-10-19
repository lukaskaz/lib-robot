cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(roarm)

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
add_dependencies(${PROJECT_NAME} libclimenu)
add_dependencies(${PROJECT_NAME} libnlohmann)
add_dependencies(${PROJECT_NAME} libhttp)
add_dependencies(${PROJECT_NAME} libtts)
add_dependencies(${PROJECT_NAME} libshellcmd)

target_link_libraries(${PROJECT_NAME}
    climenu
    http
    tts
    shellcmd
    ${Boost_LIBRARIES}
)
