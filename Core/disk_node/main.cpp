// Punto de entrada del Disk Node
#include "disk_node.h"
#include <iostream>

int main() {
    try {
        DiskNode node("../Core/config/disk1.xml");
        node.initializeStorage();
        node.printInfo();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}