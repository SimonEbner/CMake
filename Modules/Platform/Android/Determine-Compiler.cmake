#=============================================================================
# Copyright 2015-2016 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# This module is shared by multiple languages; use include blocker.
if(__ANDROID_DETERMINE_COMPILER)
  return()
endif()
set(__ANDROID_DETERMINE_COMPILER 1)

# Support for NVIDIA Nsight Tegra Visual Studio Edition was previously
# implemented in the CMake VS IDE generators.  Avoid interfering with
# that functionality for now.  Later we may try to integrate this.
if(CMAKE_VS_PLATFORM_NAME STREQUAL "Tegra-Android")
  macro(__android_determine_compiler lang)
  endmacro()
  return()
endif()

# Commonly used Android toolchain files that pre-date CMake upstream support
# set CMAKE_SYSTEM_VERSION to 1.  Avoid interfering with them.
if(CMAKE_SYSTEM_VERSION EQUAL 1)
  macro(__android_determine_compiler lang)
  endmacro()
  return()
endif()

# Identify the host platform.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set(_ANDROID_HOST_EXT "")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(_ANDROID_HOST_EXT "")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(_ANDROID_HOST_EXT ".exe")
else()
  message(FATAL_ERROR "Android: Builds hosted on '${CMAKE_HOST_SYSTEM_NAME}' not supported.")
endif()

if(CMAKE_ANDROID_NDK)
  include(Platform/Android/Determine-Compiler-NDK)
elseif(CMAKE_ANDROID_STANDALONE_TOOLCHAIN)
  include(Platform/Android/Determine-Compiler-Standalone)
else()
  set(_ANDROID_TOOL_C_COMPILER "")
  set(_ANDROID_TOOL_C_TOOLCHAIN_VERSION "")
  set(_ANDROID_TOOL_C_COMPILER_EXTERNAL_TOOLCHAIN "")
  set(_ANDROID_TOOL_C_TOOLCHAIN_PREFIX "")
  set(_ANDROID_TOOL_C_TOOLCHAIN_SUFFIX "")
  set(_ANDROID_TOOL_CXX_COMPILER "")
  set(_ANDROID_TOOL_CXX_TOOLCHAIN_VERSION "")
  set(_ANDROID_TOOL_CXX_COMPILER_EXTERNAL_TOOLCHAIN "")
  set(_ANDROID_TOOL_CXX_TOOLCHAIN_PREFIX "")
  set(_ANDROID_TOOL_CXX_TOOLCHAIN_SUFFIX "")
endif()

unset(_ANDROID_HOST_EXT)

macro(__android_determine_compiler lang)
  if(_ANDROID_TOOL_${lang}_COMPILER)
    set(CMAKE_${lang}_COMPILER "${_ANDROID_TOOL_${lang}_COMPILER}")
    set(CMAKE_${lang}_COMPILER_EXTERNAL_TOOLCHAIN "${_ANDROID_TOOL_${lang}_COMPILER_EXTERNAL_TOOLCHAIN}")

    # Save the Android-specific information in CMake${lang}Compiler.cmake.
    set(CMAKE_${lang}_COMPILER_CUSTOM_CODE "
set(CMAKE_${lang}_ANDROID_TOOLCHAIN_VERSION \"${_ANDROID_TOOL_${lang}_TOOLCHAIN_VERSION}\")
set(CMAKE_${lang}_COMPILER_EXTERNAL_TOOLCHAIN \"${_ANDROID_TOOL_${lang}_COMPILER_EXTERNAL_TOOLCHAIN}\")
set(CMAKE_${lang}_ANDROID_TOOLCHAIN_PREFIX \"${_ANDROID_TOOL_${lang}_TOOLCHAIN_PREFIX}\")
set(CMAKE_${lang}_ANDROID_TOOLCHAIN_SUFFIX \"${_ANDROID_TOOL_${lang}_TOOLCHAIN_SUFFIX}\")
")
  endif()
endmacro()
