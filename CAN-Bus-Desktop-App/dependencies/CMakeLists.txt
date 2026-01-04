include(FetchContent)

FetchContent_Declare(
    sfml
    GIT_REPOSITORY "https://github.com/SFML/SFML"
    GIT_TAG 7f1162dfea4969bc17417563ac55d93b72e84c1e #3.0.0
)


FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui"
    GIT_TAG 5c1d2d1e4c562a2ed3efbc64476e703a655b45fd #1.91.7
)

FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY "https://github.com/SFML/imgui-sfml"
    GIT_TAG 5e8f8fb857c8f3cff1096bf3e4402c64e3bcf6af #3.0
)

add_subdirectory(sfml)
add_subdirectory(imgui-sfml)
