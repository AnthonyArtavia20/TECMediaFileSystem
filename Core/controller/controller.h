#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <vector>
#include <unordered_map>

class Raid5Controller {
public:
    Raid5Controller(size_t blockSize);

    //Operaciones internas lógica c++
    void storeFile(const std::string& filepath);
    bool recoverMissingBlocks(const std::string& filename);
    bool rebuildPdfFromDisks(const std::string& filename, const std::string& outputFilename);
    void setBlockSize(size_t newBlockSize);

    //métodos para saber el estado del RAID
    void startDisk(int disk_id);
    void stopDisk(int disk_id);
    void setDiskState(int id, bool active);
    bool getDiskState(int id) const;
    int getActiveDiskCount() const;
    int getDiskCount() const;

private:
    size_t blockSize;
    std::vector<std::string> nodes;
    std::string filename;
    std::unordered_map<int, bool> diskStates;
};

#endif