enable_language(C)
enable_language(CXX)

if(NOT ANDROID)
  message(SEND_ERROR "CMake variable 'ANDROID' is not set to a true value.")
endif()

foreach(f
    "${CMAKE_C_ANDROID_TOOLCHAIN_PREFIX}gcc${CMAKE_C_ANDROID_TOOLCHAIN_SUFFIX}"
    "${CMAKE_CXX_ANDROID_TOOLCHAIN_PREFIX}g++${CMAKE_CXX_ANDROID_TOOLCHAIN_SUFFIX}"
    "${CMAKE_CXX_ANDROID_TOOLCHAIN_PREFIX}cpp${CMAKE_CXX_ANDROID_TOOLCHAIN_SUFFIX}"
    "${CMAKE_CXX_ANDROID_TOOLCHAIN_PREFIX}ar${CMAKE_CXX_ANDROID_TOOLCHAIN_SUFFIX}"
    "${CMAKE_CXX_ANDROID_TOOLCHAIN_PREFIX}ld${CMAKE_CXX_ANDROID_TOOLCHAIN_SUFFIX}"
    )
  if(NOT EXISTS "${f}")
    message(SEND_ERROR "Expected file does not exist:\n \"${f}\"")
  endif()
endforeach()

string(APPEND CMAKE_C_FLAGS " -Werror")
string(APPEND CMAKE_CXX_FLAGS " -Werror")
string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,-no-undefined")

if(CMAKE_ANDROID_NDK)
  if(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION MATCHES "clang")
    add_definitions(-DCOMPILER_IS_CLANG)
  endif()
elseif(CMAKE_ANDROID_STANDALONE_TOOLCHAIN)
  execute_process(
    COMMAND ${CMAKE_ANDROID_STANDALONE_TOOLCHAIN}/bin/clang --version
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
    RESULT_VARIABLE _res
    )
  if(_res EQUAL 0)
    add_definitions(-DCOMPILER_IS_CLANG)
  endif()
endif()

if(CMAKE_ANDROID_STL_TYPE STREQUAL "none")
  add_definitions(-DSTL_NONE)
elseif(CMAKE_ANDROID_STL_TYPE STREQUAL "system")
  add_definitions(-DSTL_SYSTEM)
elseif(CMAKE_ANDROID_STL_TYPE MATCHES [[^gabi\+\+]])
  add_definitions(-DSTL_GABI)
endif()

string(REPLACE "-" "_" abi "${CMAKE_ANDROID_ARCH_ABI}")
add_definitions(-DABI_${abi})
add_definitions(-DAPI_LEVEL=${CMAKE_SYSTEM_VERSION})
if(CMAKE_ANDROID_ARCH_ABI MATCHES "^armeabi")
  add_definitions(-DARM_MODE=${CMAKE_ANDROID_ARM_MODE})
  message(STATUS "CMAKE_ANDROID_ARM_MODE=${CMAKE_ANDROID_ARM_MODE}")
endif()
if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
  add_definitions(-DARM_NEON=${CMAKE_ANDROID_ARM_NEON})
  message(STATUS "CMAKE_ANDROID_ARM_NEON=${CMAKE_ANDROID_ARM_NEON}")
endif()
add_executable(android_c android.c)
add_executable(android_cxx android.cxx)
