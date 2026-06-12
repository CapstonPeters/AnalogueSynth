# Fetch JUCE 8 from GitHub
include(FetchContent)

# Disable JUCE utilities to avoid juceaide issues
set(JUCE_BUILD_UTILITIES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.0
    GIT_SHALLOW    ON
)

FetchContent_MakeAvailable(JUCE)

# Add JUCE to the project
add_subdirectory(${juce_SOURCE_DIR} ${juce_BINARY_DIR} EXCLUDE_FROM_ALL)