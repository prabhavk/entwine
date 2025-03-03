#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <map>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Node.h"


// Network Class
class Network {
private:
    std::map<std::string, std::unique_ptr<Node>> nodes; // Map storing nodes with unique_ptr

public:
    // Constructor
    Network() = default;

    // Disable copy constructor & copy assignment (to enforce unique ownership)
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    // Default move constructor & move assignment
    Network(Network&&) = default;
    Network& operator=(Network&&) = default;

    // Function to add a node
    void addNode(const std::string& node_id, const std::string& date) {
        if (nodes.find(node_id) != nodes.end()) {
            std::cerr << "Error: Node with ID '" << node_id << "' already exists.\n";
            return;
        }
        nodes[node_id] = std::make_unique<Node>(node_id, date);
    }

    // Function to get a node (returns pointer or nullptr if not found)
    Node* getNode(const std::string& node_id) const {
        auto it = nodes.find(node_id);
        if (it != nodes.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // Function to remove a node
    void removeNode(const std::string& node_id) {
        if (nodes.erase(node_id) == 0) {
            std::cerr << "Error: Node with ID '" << node_id << "' does not exist.\n";
        }
    }

    // Function to read mutation file
    void ReadMutationFile(const std::string& filename) {
        std::ifstream file(filename);

        if (!file) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
        }

        std::string line;
        bool firstLine = true; // Skip header
        while (std::getline(file, line)) {
            if (firstLine) { // Place outside while loop
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
                nodes[id] = std::make_unique<Node>(id, date, mutations);
                // graft 
            }
        }

        file.close();
    }

    // Function to print all nodes in the network
    void printNetwork() const {
        std::cout << "Network Nodes:\n";
        for (const auto& pair : nodes) {
            std::cout << "Node ID: " << pair.first << ", Date: " << pair.second->getDate() << std::endl;
        }
    }
};

#endif // NETWORK_H
