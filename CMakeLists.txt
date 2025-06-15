cmake_minimum_required(VERSION 3.15)

project(TECMediaFileSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------- Disk Node -----------------
file(GLOB_RECURSE DISK_SOURCES
    Core/disk_node/*.cpp
)
include_directories(Core/disk_node)
add_executable(TECMFS-Disk ${DISK_SOURCES})

# -------- Controller Node -----------------
file(GLOB_RECURSE CTRL_SOURCES
    Core/controller/*.cpp
)
include_directories(Core/controller)
add_executable(TECMFS-Controller ${CTRL_SOURCES})

# ---------- pugixml ---------------
include_directories(/usr/local/opt/pugixml/include)
link_directories(/usr/local/opt/pugixml/lib)

set(PUGIXML_LIB /usr/local/opt/pugixml/lib/libpugixml.dylib)

target_link_libraries(TECMFS-Disk ${PUGIXML_LIB})
target_link_libraries(TECMFS-Controller ${PUGIXML_LIB})
