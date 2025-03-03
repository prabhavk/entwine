#ifndef PARSER_H
#define PARSER_H

#include "Node.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// Function to read mutations from CSV file
std::vector<Node> readMutationFile(const std::string& filename) {
    std::vector <Node> nodes;
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return nodes;
    }

    std::string line;
    bool firstLine = true; // Skip header
    while (std::getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            continue;
        }

        std::istringstream ss(line);
        std::string id, date, mutationString;
        std::vector<std::string> mutations;

        if (std::getline(ss, date, ',') && std::getline(ss, id, ',')) {
            while (std::getline(ss, mutationString, ',')) {
                mutations.push_back(mutationString);
            }
            nodes.push_back(Node(id, date, mutations));
        }
    }

    file.close();
    return nodes;
}

#endif // PARSER_H
