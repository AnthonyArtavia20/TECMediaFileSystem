cmake_minimum_required(VERSION 3.15)
project(TECMediaFileSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------- Disk Node -----------------
file(GLOB_RECURSE DISK_SOURCES Core/disk_node/*.cpp)
add_executable(TECMFS-Disk ${DISK_SOURCES})
target_include_directories(TECMFS-Disk PRIVATE Core/disk_node)

# -------- Controller Node -----------------
file(GLOB_RECURSE CTRL_SOURCES Core/controller/*.cpp Core/http/*.cpp)
add_executable(TECMFS-Controller ${CTRL_SOURCES})
target_include_directories(TECMFS-Controller PRIVATE Core/controller Core/http)

# -------- pugixml --------
if(APPLE)
    # macOS: usar ruta típica de Homebrew
    message(STATUS "Detected macOS system")
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
    target_link_libraries(TECMFS-Disk pugixml)
    target_link_libraries(TECMFS-Controller pugixml)

elseif(UNIX)
    # Linux
    message(STATUS "Detected Linux system")
    find_path(PUGIXML_INCLUDE_DIR pugixml.hpp)
    find_library(PUGIXML_LIBRARY NAMES pugixml)

    if (PUGIXML_INCLUDE_DIR AND PUGIXML_LIBRARY)
        target_include_directories(TECMFS-Disk PRIVATE ${PUGIXML_INCLUDE_DIR})
        target_link_libraries(TECMFS-Disk ${PUGIXML_LIBRARY})
        target_include_directories(TECMFS-Controller PRIVATE ${PUGIXML_INCLUDE_DIR})
        target_link_libraries(TECMFS-Controller ${PUGIXML_LIBRARY})
    else()
        message(FATAL_ERROR "pugixml not found. Please install it with your package manager.")
    endif()
endif()
