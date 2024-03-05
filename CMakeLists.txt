# CMakeList.txt : CMake project for TDDLinux, include source and define
# project specific logic here.
#
option(MAKE_SO "make the so" OFF)
set(CMAKE_CXX_FLAGS "-Wall")

cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TDDLinux")
set(CORE_TARGET_NAME "TDDLinux")
set(CORE_TARGETS "")
# Add source to this project's executable.

if (MAKE_SO)
	add_library(TDDLinux SHARED "TDDLinux.cpp")
else()
	add_executable (TDDLinux "TDDLinux.cpp" "TDDLinux.h")
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TDDLinux PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
find_package(xtl REQUIRED)
include_directories(${XTL_INCLUDE_DIRS})
find_package(xtensor REQUIRED)
include_directories(${XTENSOR_INCLUDE_DIRS})


add_subdirectory(operations)
add_subdirectory(parsers)
add_subdirectory(dd)


target_link_libraries(${PROJECT_NAME} xtl xtensor)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)


