cmake_minimum_required(VERSION 3.10)

find_package(Boost COMPONENTS program_options REQUIRED)
include(ExternalProject)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/libclimenu-src")
set(build_dir "${CMAKE_BINARY_DIR}/libclimenu-build")

EXTERNALPROJECT_ADD(
  libclimenu
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-climenu.git
  GIT_TAG           main
  PATCH_COMMAND     ${patching_cmd}
  PREFIX            libclimenu-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    ${source_dir} && make -j $(nproc)
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/libhttp-src")
set(build_dir "${CMAKE_BINARY_DIR}/libhttp-build")

EXTERNALPROJECT_ADD(
  libhttp
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-http.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libhttp-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    ${source_dir} && make
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/libtts-src")
set(build_dir "${CMAKE_BINARY_DIR}/libtts-build")
set(vcpkg_dir "${CMAKE_BINARY_DIR}/libtts-build/build/vcpkg_installed")

EXTERNALPROJECT_ADD(
  libtts
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-tts.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libtts-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build &&
                    cmake -DBUILD_SHARED_LIBS=ON ${source_dir} && make
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)
link_directories(${vcpkg_dir}/lib)

set(source_dir "${CMAKE_BINARY_DIR}/libshellcmd-src")
set(build_dir "${CMAKE_BINARY_DIR}/libshellcmd-build")

EXTERNALPROJECT_ADD(
  libshellcmd
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-shellcmd.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libshellcmd-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    ${source_dir} && make
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/liblogger-src")
set(build_dir "${CMAKE_BINARY_DIR}/liblogger-build")

EXTERNALPROJECT_ADD(
  liblogger
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-logger.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            liblogger-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    ${source_dir} && make
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)
