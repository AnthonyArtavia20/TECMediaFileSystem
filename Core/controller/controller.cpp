#include "controller.h"
#include "../../Core/pdfToBit/Pdfabits.cpp"

#include <iostream>

#include <pugixml.hpp>
#include <filesystem>

Raid5Controller::Raid5Controller(size_t blockSize) : blockSize(blockSize) {
    std::vector<std::string> configFiles = {
    "../Core/config/disk1.xml", "../Core/config/disk2.xml",
    "../Core/config/disk3.xml", "../Core/config/disk4.xml"
    };


    for (const auto& file : configFiles) {
        pugi::xml_document doc;
        if (!doc.load_file(file.c_str())) {
            throw std::runtime_error("No se pudo cargar el archivo: " + file);
        }
        std::string path = doc.child("DiskNode").child("StoragePath").text().as_string();
        nodes.push_back(path);
    }
}


void Raid5Controller::storeFile(const std::string& filename) {
    auto data = PdfaBit::readFile(filename);
    auto blocks = PdfaBit::partitionIntoBlocks(data, blockSize);

    int dataDisks = nodes.size() - 1;
    int stripeCount = (blocks.size() + dataDisks - 1) / dataDisks;
    int blockIndex = 0;

    //separacion en stripes para la reparticion entre los disk nodes
    for (int stripe = 0; stripe < stripeCount; ++stripe) {
        std::vector<std::vector<uint8_t>> stripeData(dataDisks);
        int parityDisk = stripe % nodes.size();

        for (int i = 0; i < dataDisks && blockIndex < blocks.size(); ++i) {
            stripeData[i] = blocks[blockIndex++];
        }

        //calculo de paridad
        std::vector<uint8_t> parity(blockSize, 0);
        for (const auto& blk : stripeData) {
            for (size_t j = 0; j < blk.size(); ++j)
                parity[j] ^= blk[j];
        }

        //control de la cantidad de disk nodes funcionando
        int diskCounter = 0;
        for (int i = 0; i < nodes.size(); ++i) {
            if (i == parityDisk) {
                PdfaBit::writeBlock(nodes[i], stripe, parity);
            } else if (diskCounter < dataDisks && stripeData[diskCounter].size() > 0) {
                PdfaBit::writeBlock(nodes[i], stripe, stripeData[diskCounter++]);
            }
        }
    }

    std::cout << "Archivo almacenado usando RAID 5." << std::endl;
}
