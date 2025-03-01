#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>


using namespace std;
// Class to store mutation data
class Node {
public:
    string date;
    string id;
    unordered_map<string, vector<string> > mutations;
    
    Node(string date, string id) : date(date), id(id) {}
    
    void addMutations(string segment, string mutationStr) {
        vector<string> mutationList;
        stringstream ss(mutationStr);
        string mutation;
        while (getline(ss, mutation, ':')) {
            mutationList.push_back(mutation);
        }
        mutations[segment] = mutationList;
    }

    void print() {
        cout << "Sample ID: " << id << " | Date: " << date << "\n";
        for (const auto& pair : mutations) {
            cout << pair.first << ": ";
            for (const auto& mut : pair.second) {
                cout << mut << " ";
            }
            cout << "\n";
        }
        cout << "---------------------\n";
    }
};

// Function to parse CSV file
vector<Node> parseCSV(const string& filename) {
    vector<Node> nodes;
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Could not open file " << filename << "\n";
        return nodes;
    }
    
    string line, word;
    vector<string> headers;
    
    // Read header line
    if (getline(file, line)) {
        stringstream ss(line);
        while (getline(ss, word, ',')) {
            headers.push_back(word);
        }
    }
    
    // Read data lines
    while (getline(file, line)) {
        stringstream ss(line);
        string date, id;
        getline(ss, date, ',');
        getline(ss, id, ',');
        
        Node node(date, id);
        
        for (size_t i = 2; i < headers.size(); ++i) {
            if (getline(ss, word, ',')) {
                if (!word.empty()) {
                    node.addMutations(headers[i], word);
                }
            }
        }
        nodes.push_back(node);
    }
    return nodes;
}

int main(int argc, char* argv[]) {
    if (argc < 3 || string(argv[1]) != "--mutations") {
        cerr << "Usage: " << argv[0] << " --mutations <file.csv>\n";
        return 1;
    }
    
    string filename = argv[2];
    vector<Node> nodes = parseCSV(filename);
    
    for (const auto& node : nodes) {
        node.print();
    }
    return 0;
}
