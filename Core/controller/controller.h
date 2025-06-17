#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <vector>

class Raid5Controller {
public:
    Raid5Controller(size_t blockSize);
    void storeFile(const std::string& filename);
    bool recoverMissingBlocks();
    bool rebuildOriginalFile(const std::string& outputPath);
    bool rebuildPdfFromDisks(const std::string& outputFilename);


private:
    size_t blockSize;
    std::vector<std::string> nodes;
};

#endif