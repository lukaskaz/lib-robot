cmake_minimum_required(VERSION 3.10)

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
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

set(source_dir "${CMAKE_BINARY_DIR}/libnlohmann-src")
set(build_dir "${CMAKE_BINARY_DIR}/libnlohmann-build")

EXTERNALPROJECT_ADD(
  libnlohmann
  GIT_REPOSITORY    https://github.com/nlohmann/json.git
  GIT_TAG           v3.11.3
  PATCH_COMMAND     ${patching_cmd}
  PREFIX            libnlohmann-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

include_directories(${source_dir}/include)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/libcpr-src")
set(build_dir "${CMAKE_BINARY_DIR}/libcpr-build")

EXTERNALPROJECT_ADD(
  libcpr
  GIT_REPOSITORY    https://github.com/libcpr/cpr.git
  GIT_TAG           1.10.5
  PATCH_COMMAND     ${patching_cmd}
  PREFIX            libcpr-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D CPR_USE_SYSTEM_CURL=ON
                    ${source_dir} && make
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

include_directories(${source_dir}/include)
include_directories(${build_dir}/build/cpr_generated_includes)
link_directories(${build_dir}/build/lib)

set(source_dir "${CMAKE_BINARY_DIR}/libtts-src")
set(build_dir "${CMAKE_BINARY_DIR}/libtts-build")

EXTERNALPROJECT_ADD(
  libtts
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-tts.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libtts-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake 
    -DCMAKE_TOOLCHAIN_FILE=$ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake 
    -DBUILD_SHARED_LIBS=ON ${source_dir} && make
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

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
