cmake_minimum_required(VERSION 3.20)

# vcpkg ツールチェーンの自動フォールバックロード
if(NOT CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain")
  elseif(EXISTS "C:/UserFiles/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "C:/UserFiles/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain")
  endif()
endif()


# 1. vcpkgの設定は最優先
set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "vcpkg target triplet")

# 2. ポリシー設定
cmake_policy(SET CMP0091 NEW)

# 3. プロジェクト定義
project(AGS_2026_Summer LANGUAGES C CXX)

# 4. 構成定義
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Editor" CACHE STRING "Available build configurations" FORCE)

# 5. MSVCランタイムの設定
set(CMAKE_MSVC_RUNTIME_LIBRARY_DEBUG  "MultiThreadedDebugDLL")
set(CMAKE_MSVC_RUNTIME_LIBRARY_RELEASE "MultiThreadedDLL")
set(CMAKE_MSVC_RUNTIME_LIBRARY_EDITOR  "MultiThreadedDebugDLL")

# C++規格の設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC 固有の設定
if(MSVC)
  # 初期化直後のDEBUGフラグをEDITORに複製
  set(CMAKE_CXX_FLAGS_EDITOR "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "Flags used by the CXX compiler during Editor builds." FORCE)
  set(CMAKE_C_FLAGS_EDITOR "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "Flags used by the C compiler during Editor builds." FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_EDITOR "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "Flags used by the linker during Editor builds." FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS_EDITOR "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING "Flags used by the shared linker during Editor builds." FORCE)
  set(CMAKE_STATIC_LINKER_FLAGS_EDITOR "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}" CACHE STRING "Flags used by the static linker during Editor builds." FORCE)
endif()

# DxLib の環境変数処理
if(DEFINED ENV{DXLIB_DIR})
  set(DXLIB_DIR "$ENV{DXLIB_DIR}")
  string(REPLACE "\\" "/" DXLIB_DIR "${DXLIB_DIR}")
  message(STATUS "DxLib Directory: ${DXLIB_DIR}")
else()
  message(FATAL_ERROR "Environment variable DXLIB_DIR is not defined. Please set DXLIB_DIR to your DxLib directory.")
endif()

# 外部パッケージの検索
find_package(cpr CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# enet の検出
find_path(ENET_INCLUDE_DIR NAMES enet/enet.h)
find_library(ENET_LIBRARY NAMES enet)

if(ENET_INCLUDE_DIR AND ENET_LIBRARY)
  add_library(enet::enet UNKNOWN IMPORTED)
  set_target_properties(enet::enet PROPERTIES
    IMPORTED_LOCATION "${ENET_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${ENET_INCLUDE_DIR}"
  )
  message(STATUS "Found enet: ${ENET_LIBRARY}")
else()
  message(FATAL_ERROR "Could NOT find enet. Please verify your vcpkg installation.")
endif()

# 各プロジェクトの追加
add_subdirectory(BroccoliEngine)
add_subdirectory(Game)