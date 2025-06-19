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
    message(STATUS "Detected macOS system")
    include_directories(/usr/local/opt/pugixml/include)
    link_directories(/usr/local/opt/pugixml/lib)
    set(PUGIXML_LIB /usr/local/opt/pugixml/lib/libpugixml.dylib)

    # Unificar aquí
    target_link_libraries(TECMFS-Disk PRIVATE ${PUGIXML_LIB})
    target_link_libraries(TECMFS-Controller
        PRIVATE
            ${PUGIXML_LIB}
    )

elseif(UNIX)
    message(STATUS "Detected Linux system")
    find_path(PUGIXML_INCLUDE_DIR pugixml.hpp)
    find_library(PUGIXML_LIBRARY NAMES pugixml)

    if (PUGIXML_INCLUDE_DIR AND PUGIXML_LIBRARY)
        target_include_directories(TECMFS-Disk PRIVATE ${PUGIXML_INCLUDE_DIR})
        target_link_libraries(TECMFS-Disk PRIVATE ${PUGIXML_LIBRARY})
        target_include_directories(TECMFS-Controller PRIVATE ${PUGIXML_INCLUDE_DIR})
        
        # Unificar aquí también
        target_link_libraries(TECMFS-Controller
            PRIVATE
                ${PUGIXML_LIBRARY}
        )
    else()
        message(FATAL_ERROR "pugixml not found. Please install it with your package manager.")
    endif()
endif()

# -------- nlohmann_json --------
find_package(nlohmann_json REQUIRED)
target_link_libraries(TECMFS-Controller
    PRIVATE
        nlohmann_json::nlohmann_json
)
