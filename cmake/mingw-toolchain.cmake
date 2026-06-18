# MinGW-w64 x86_64 toolchain for cross-compiling Windows VST3 on Linux
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# Prevent CMake from trying to run host programs during cross-compile
set(CMAKE_CROSSCOMPILING TRUE)

# Target environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Windows SDK paths (adjust if needed)
set(WINDOWS_SDK_DIR "/usr/x86_64-w64-mingw32")
set(CMAKE_FIND_ROOT_PATH ${WINDOWS_SDK_DIR})

# Disable Direct2D (not available in MinGW headers) - use software renderer
add_compile_definitions(JUCE_DIRECT2D=0)

# JUCE needs these for Windows
set(JUCE_MINGW 1)
set(JUCE_WINDOWS 1)

# Thread model
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_GLIBCXX_USE_CXX11_ABI=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

# Release optimizations
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -ffast-math")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -ffast-math")

# VST3 SDK - JUCE fetches its own, but we can hint
set(VST3_SDK_DIR "${CMAKE_FIND_ROOT_PATH}/vst3sdk")

# Linker flags for Windows
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# Prevent picking up host libraries
set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".dll.a" ".lib")