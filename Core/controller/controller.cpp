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

    int totalDisks = nodes.size();
    int dataDisks = totalDisks - 1;
    int stripeCount = (blocks.size() + dataDisks - 1) / dataDisks;
    int blockIndex = 0;

    for (int stripe = 0; stripe < stripeCount; ++stripe) {
        int parityDisk = stripe % totalDisks;
        std::vector<std::vector<uint8_t>> stripeData(totalDisks, std::vector<uint8_t>(blockSize, 0));

        // Asignar bloques a los discos (excepto el de paridad)
        for (int i = 0; i < totalDisks; ++i) {
            if (i == parityDisk || blockIndex >= blocks.size()) continue;
            stripeData[i] = blocks[blockIndex++];
        }

        // Calcular paridad XOR de los bloques presentes
        std::vector<uint8_t> parity(blockSize, 0);
        for (int i = 0; i < totalDisks; ++i) {
            if (i == parityDisk || stripeData[i].empty()) continue;
            for (size_t j = 0; j < blockSize; ++j) {
                parity[j] ^= stripeData[i][j];
            }
        }
        stripeData[parityDisk] = parity;

        // Guardar todos los bloques del stripe
        for (int i = 0; i < totalDisks; ++i) {
            PdfaBit::writeBlock(nodes[i], stripe, stripeData[i]);
        }
    }

    std::cout << "Archivo almacenado usando RAID 5." << std::endl;
}

bool Raid5Controller::recoverMissingBlocks() {
    bool reconstructedBlock = false;

    int totalDisks = nodes.size();
    int maxStripes = 1000; 

    for (int stripe = 0; stripe < maxStripes; ++stripe) {
        int missingIndex = -1;
        std::vector<std::vector<uint8_t>> presentBlocks(totalDisks);

        for (int i = 0; i < totalDisks; ++i) {
            try {
                presentBlocks[i] = PdfaBit::readBlock(nodes[i], stripe);
                if (presentBlocks[i].empty()) missingIndex = i;
            } catch (...) {
                missingIndex = i;
            }
        }

        // Verificamos si falta un único bloque
        int missingCount = 0;
        for (const auto& b : presentBlocks) {
            if (b.empty()) ++missingCount;
        }

        if (missingCount == 1 && missingIndex != -1) {
            std::cout << "Recuperando bloque faltante en stripe " << stripe << " del nodo " << missingIndex << std::endl;

            // Construimos vector sin el bloque faltante
            std::vector<std::vector<uint8_t>> available;
            for (int i = 0; i < totalDisks; ++i) {
                if (i != missingIndex) available.push_back(presentBlocks[i]);
            }

            auto recovered = PdfaBit::recoverBlockUsingParity(available);
            PdfaBit::writeBlock(nodes[missingIndex], stripe, recovered);

            // se indica que se reconstruyó un bloque
            reconstructedBlock = true;
        }
    }

    return reconstructedBlock;
}

bool Raid5Controller::rebuildPdfFromDisks(const std::string& outputFilename) {
    int totalDisks = nodes.size();
    int maxStripes = 17; 
    std::vector<uint8_t> fullData;

    for (int stripe = 0; stripe < maxStripes; ++stripe) {
        int missingIndex = -1;
        std::vector<std::vector<uint8_t>> presentBlocks(totalDisks);

        // Leer todos los bloques del stripe
        for (int i = 0; i < totalDisks; ++i) {
            try {
                presentBlocks[i] = PdfaBit::readBlock(nodes[i], stripe);
                if (presentBlocks[i].empty()) missingIndex = i;
            } catch (...) {
                missingIndex = i;
            }
        }

        // Contar los bloques faltantes
        int missingCount = 0;
        for (const auto& b : presentBlocks) {
            if (b.empty()) ++missingCount;
        }

        if (missingCount > 1) {
            std::cerr << "No se puede reconstruir el stripe " << stripe << ": faltan múltiples bloques." << std::endl;
            return false;
        }

        std::vector<uint8_t> dataBlock;

        if (missingCount == 1 && missingIndex != -1) {
            std::vector<std::vector<uint8_t>> available;
            for (int i = 0; i < totalDisks; ++i) {
                if (i != missingIndex) available.push_back(presentBlocks[i]);
            }
            dataBlock = PdfaBit::recoverBlockUsingParity(available);
            std::cerr << "Bloque reconstruido en stripe " << stripe << " desde paridad." << std::endl;
        } else {
            // Tomar el bloque de datos 
            for (int i = 0; i < totalDisks; ++i) {
                if (!presentBlocks[i].empty()) {
                    dataBlock = presentBlocks[i];
                    break;
                }
            }
        }

        fullData.insert(fullData.end(), dataBlock.begin(), dataBlock.end());
    }

    // Se reconstruye el archivo
    std::ofstream out(outputFilename, std::ios::binary);
    if (!out) {
        std::cerr << "No se pudo crear el archivo de salida." << std::endl;
        return false;
    }
    out.write(reinterpret_cast<const char*>(fullData.data()), fullData.size());
    out.close();

    std::cout << "Archivo reconstruido exitosamente en: " << outputFilename << std::endl;
    return true;
}
