message (STATUS "Fetching SFML...")

# No need to build audio
set(SFML_BUILD_AUDIO FALSE)

FetchContent_MakeAvailable(sfml)
