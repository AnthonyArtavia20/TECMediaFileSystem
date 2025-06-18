// Punto de entrada del Disk Node
#include "disk_node.h"
#include <filesystem>
#include <iostream>
using namespace std;

int main() {
    try {

        string basePath = filesystem::current_path().string(); //para obtener la ruta actual donde se ejcuta el programa


        // creacion de los disk nodes donde se almacenaran los bits
        // Construir rutas absolutas a los XML
        Disk_Node disk_node1(basePath + "/Core/config/disk1.xml");
        disk_node1.initializeStorage();

        Disk_Node disk_node2(basePath + "/Core/config/disk2.xml");
        disk_node2.initializeStorage();

        Disk_Node disk_node3(basePath + "/Core/config/disk3.xml");
        disk_node3.initializeStorage();

        Disk_Node disk_node4(basePath + "/Core/config/disk4.xml");
        disk_node4.initializeStorage();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}