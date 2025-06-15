// Punto de entrada del Disk Node
#include "disk_node.h"
#include <iostream>

int main() {
    try {

        // creacion de los disk nodes donde se almacenaran los bits
        Disk_Node disk_node1("../Core/config/disk1.xml");
        disk_node1.initializeStorage();
        disk_node1.printInfo();
        Disk_Node disk_node2("../Core/config/disk2.xml");
        disk_node2.initializeStorage();
        Disk_Node disk_node3("../Core/config/disk3.xml");
        disk_node3.initializeStorage();
        Disk_Node disk_node4("../Core/config/disk4.xml");
        disk_node4.initializeStorage();
        disk_node4.printInfo();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}