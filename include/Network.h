#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

// Helper function to trim whitespace from a string
inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

// Node Class
class Node {
private:
    std::string id;
    std::string date;
    std::map<std::string, std::set<std::string>> mutations; // Mutations by segment

public:
    // Constructor
    Node(const std::string& node_id, const std::string& node_date, 
         const std::map<std::string, std::set<std::string>>& node_mutations)
        : id(node_id), date(node_date), mutations(node_mutations) {}

    // Getters
    std::string getId() const { return id; }
    std::string getDate() const { return date; }
    std::map<std::string, std::set<std::string>> getMutations() const { return mutations; }

    // Print node details
    void printNode() const {
        std::cout << "ID: " << id << ", Date: " << date << ", Mutations: ";
        for (const auto& [segment, mut_set] : mutations) {
            std::cout << segment << ":[";
            for (auto it = mut_set.begin(); it != mut_set.end(); ++it) {
                std::cout << *it;
                if (std::next(it) != mut_set.end()) std::cout << ", ";
            }
            std::cout << "] ";
        }
        std::cout << std::endl;
    }
};

// Network Class
class Network {
private:
    std::map<std::string, std::unique_ptr<Node>> nodes; // Map of nodes
    std::vector<std::string> segments; // Stores segment names from file header

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
    void addNode(const std::string& node_id, const std::string& date, 
                 const std::map<std::string, std::set<std::string>>& mutations) {
        if (nodes.find(node_id) != nodes.end()) {
            std::cerr << "Error: Node with ID '" << node_id << "' already exists.\n";
            return;
        }
        nodes[node_id] = std::make_unique<Node>(node_id, date, mutations);
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

    // Function to print all nodes in the network
    void printNetwork() const {
        std::cout << "Network Nodes:\n";
        for (const auto& pair : nodes) {
            pair.second->printNode();
        }
    }

    // Function to load mutations from a CSV file
    void loadMutationsFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return;
        }

        std::string line;
        bool firstLine = true;

        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;

            // Tokenize line by ","
            while (std::getline(ss, token, ',')) {
                tokens.push_back(trim(token));  // Remove whitespace
            }

            if (firstLine) {
                // Store segment names from header (excluding Date and ID columns)
                for (size_t i = 2; i < tokens.size(); i++) {
                    segments.push_back(tokens[i]);
                }
                firstLine = false;
                continue;
            }

            if (tokens.size() < 2) continue; // Skip invalid lines

            std::string date = trim(tokens[0]);
            std::string node_id = trim(tokens[1]);
            std::map<std::string, std::set<std::string>> mutations;

            // Parse mutations for each segment
            for (size_t i = 2; i < tokens.size(); i++) {
                std::istringstream mutationStream(tokens[i]);
                std::string mutation;
                std::set<std::string> mutationSet;

                // Tokenize mutations by ":"
                while (std::getline(mutationStream, mutation, ':')) {
                    mutationSet.insert(trim(mutation));  // Remove whitespace
                }

                if (!segments.empty() && (i - 2) < segments.size()) {
                    mutations[segments[i - 2]] = mutationSet; // Map mutations to correct segment
                }
            }

            // Add node to network
            addNode(node_id, date, mutations);
        }

        file.close();
    }
};

#endif // NETWORK_H
