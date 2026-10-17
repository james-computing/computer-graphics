cmake_minimum_required(VERSION 4.2.3)

project(ComputerGraphics)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app)

# Store source file paths into SOURCE_FILES variable, so we don't need to type all paths manually.
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")
# Print the source file paths
#[[
message("Source files:")
foreach(path IN LISTS SOURCE_FILES)
    message("\t${path}")
endforeach()
]]

# Add source files
target_sources(app
    PRIVATE
        ${SOURCE_FILES}
)

# Add directories with header files
target_include_directories(app
    PRIVATE
        include
        libraries/stb
        libraries/tinyobjloader
        #libraries/tinygltfloader
        $ENV{VULKAN_SDK}/include
)

# Add directories with libraries installed locally
target_link_directories(app
    PRIVATE
        "$ENV{VULKAN_SDK}/lib"
)

target_link_libraries(app
    PRIVATE
        glfw
        dl
        pthread
        X11
        Xxf86vm
        Xrandr
        Xi
        vulkan
)