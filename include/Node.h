#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <string>
#include <vector>

class Node {
private:
    std::string id;
    std::string date;
    std::vector<std::string> mutations;

public:
    // Constructor
    Node(const std::string& node_id, const std::string& node_date, const std::vector<std::string>& node_mutations)
        : id(node_id), date(node_date), mutations(node_mutations) {}

    // Getters
    std::string getId() const { return id; }
    std::string getDate() const { return date; }
    std::vector<std::string> getMutations() const { return mutations; }

    // Print function
    void printNode() const {
        std::cout << "ID: " << id << ", Date: " << date << ", Mutations: ";
        for (size_t i = 0; i < mutations.size(); i++) {
            std::cout << mutations[i];
            if (i < mutations.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
};

#endif // NODE_H
