//servidor local del disco
#include "disk_node.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include "pugixml.hpp"


Disk_Node::Disk_Node(const std::string& configPath) {
    loadConfig(configPath);
    numBlocks = diskSize / blockSize;
}

void Disk_Node::loadConfig(const std::string& configPath) {
    pugi::xml_document doc;
    if (!doc.load_file(configPath.c_str())) {
        throw std::runtime_error("Error loading config.xml");
    }

    auto root = doc.child("DiskNode");
    ip = root.child("IP").text().as_string();
    port = root.child("Port").text().as_int();
    storagePath = root.child("StoragePath").text().as_string();
    blockSize = root.child("BlockSize").text().as_int();
    diskSize = root.child("DiskSize").text().as_int();
}

void Disk_Node::initializeStorage() {
    std::filesystem::create_directories(storagePath);

    for (int i = 0; i < numBlocks; ++i) {
        std::string filename = storagePath + "/block_" + std::to_string(i) + ".bin";
        if (!std::filesystem::exists(filename)) {
            std::ofstream out(filename, std::ios::binary);
            std::vector<char> block(blockSize, 0);
            out.write(block.data(), block.size());
        }
    }
}

void Disk_Node::printInfo() {
    std::cout << "IP: " << ip << "\nPort: " << port << "\nBlocks: " << numBlocks << std::endl;
}
