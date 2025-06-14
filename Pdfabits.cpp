#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

// Lee el archivo PDF en un vector de bytes
std::vector<u_int8_t> readFile(const std::string& filename){
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        throw std::runtime_error("No se pudo abrir el archivo");
    }

    //Leer hasta el final del archivo
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<u_int8_t> buffer(size);
    if (!file.read((char*)buffer.data(), size)) {
        throw std::runtime_error("No se pudo leer el archivo.");
    }

    return buffer;
}

//Guardar los datos binarios 

std::vector<std::vector<u_int8_t>> partitionIntoBlocks(const std::vector<u_int8_t>& data, size_t blockSize){
    std::vector<std::vector<u_int8_t>> blocks;
    size_t totalSize = data.size();

    for (size_t i = 0; i < totalSize; i += blockSize){
        size_t currentBlockSize = std::min(blockSize, totalSize - i);
        std::vector<u_int8_t> block(data.begin() + i, data.begin() + i + currentBlockSize);
        blocks.push_back(block);
    }

    return blocks;
}

//COnvertir nuevamente en PDF

std::vector<u_int8_t> reconstructFromBlocks(const std::vector<std::vector<u_int8_t>>&blocks) {
    std::vector<u_int8_t> data;

    for (const auto& block : blocks){
        data.insert(data.end(), block.begin(), block.end());
    }
    return data;
}

//EScribir el archivo reconstruido como PDF

void writeFile(const std::string& filename, const std::vector<u_int8_t>& data){
    std::ofstream file(filename, std::ios::binary);

    if (!file){
        throw std::runtime_error("No se pudo abrir el archivo para escribir");
    }
    file.write((const char*)data.data(), data.size());
}

bool compare(const std::vector<u_int8_t>& a, const std::vector<u_int8_t>& b){
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i] != b[i]) return false;
    return true;
}

int main() {
    try {
        std::string filename = "AAA.pdf";  // aquí pones cualquier PDF pequeño que tengas para la prueba
        auto data = readFile(filename);

        size_t blockSize = 4096;
        auto blocks = partitionIntoBlocks(data, blockSize);

        auto reconstructed = reconstructFromBlocks(blocks);

        if (compare(data, reconstructed))
            std::cout << "✅ La reconstrucción fue correcta, los datos son idénticos.\n";
        else
            std::cout << "❌ Los datos NO son iguales, algo falló.\n";

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}