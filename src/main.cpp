#include "../include/Network.h"
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    std::string filename = "";

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--mutations" && i + 1 < argc) {
            filename = argv[i + 1];
            i++;
        }
    }

    if (filename.empty()) {
        std::cerr << "Usage: entwine --mutations <filename.csv>" << std::endl;
        return 1;
    }

    Network NET;
    // Read mutation file
    NET.loadMutationsFromFile(filename);
    // std::vector<Node> nodes = readMutationFile(filename);

    // Print network
    NET.printNetwork();
    
    return 0;
}
