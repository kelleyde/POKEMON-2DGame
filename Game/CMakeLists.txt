cmake_minimum_required(VERSION 3.16)
project(game)

# Place built executables in build/<config> (e.g., build/Debug).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
# Place built shared libraries/dlls in build/<config> as well.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Build SDL3 from the local vendored directory.
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
# Build SDL3_image from the local vendored directory.
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)

# Create the game executable from all game source files.
add_executable(game
    main.cpp
    Engine.cpp
    Scene.cpp
    components.cpp
    Box.cpp
    Monster.cpp
    Player.cpp
    InputHandler.cpp
)

# Link SDL_image before/with SDL so image loading symbols resolve.
target_link_libraries(game PRIVATE
    SDL3_image::SDL3_image
    SDL3::SDL3
)

# After building the executable, copy runtime assets beside the exe.
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/sprite.png"
        "$<TARGET_FILE_DIR:game>/sprite.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/background.png"
        "$<TARGET_FILE_DIR:game>/background.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/battleground.png"
        "$<TARGET_FILE_DIR:game>/battleground.png"
)

# Allow includes like "Engine.hpp" from the project root.
target_include_directories(game PRIVATE .)


#Build commands:

# cmake -S . -B build
# cmake --build build --config Debug --target game -- /m:1
# .\build\Debug\game.exe
