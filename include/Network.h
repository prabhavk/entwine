#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>

// Helper function to trim whitespace from a string
inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

// Node Class
class Node {
    public:

    int in_degree = 0;
    int out_degree = 0;
    std::string name;
    std::string date;
    std::map<std::string, std::set<std::string>> sample_mutations; // Sample Mutations W.R.T Root Genome For Each Segment    
    std::map<std::string, std::set<std::string>> branch_mutations; // Branch Mutations For Each Segment
    Node * parent; // <-> This is usable for Non-Reassortment nodes where each segment has same parent
    std::vector<Node*> children;
    std::map<std::string,Node *> parent4seg; // <-> This is usable for Reassortment nodes where segments have different parents
    bool reassortment_node = false;
    // Constructor
    Node(const std::string& node_name, const std::string& node_date,
         const std::map<std::string, std::set<std::string>>& node_mutations)
        : name(node_name), date(node_date), sample_mutations(node_mutations) {}

    // Getters
    std::string get_name() const { return name; }
    std::string getDate() const { return date; }
    std::map<std::string, std::set<std::string>> getSampleMutations() const { return sample_mutations; }
    std::map<std::string, std::set<std::string>> getBranchMutations() const { return branch_mutations; }

    // Setters
    void set_branch_mutations(std::map<std::string, std::set<std::string>> & branch_mutations_to_set) {
        branch_mutations = branch_mutations_to_set;
    }

    void set_parent(Node* parent_to_set) {
        parent = parent_to_set;
        in_degree++;
    }

    void setParentForSegment(std::string seg_name, Node * parent_to_set) {
        parent4seg[seg_name] = parent_to_set;
    }

    void remove_parent() {
        parent = nullptr;
        in_degree --;
        branch_mutations.clear();
    }

    void add_child(Node* child_to_add) {
        children.push_back(child_to_add);
        out_degree++;
    }

    void remove_child(Node* child_to_remove) {
        children.push_back(child_to_remove);
        std::vector<Node*>::iterator child_to_remove_iterator = std::remove(children.begin(),children.end(),child_to_remove);
        if (child_to_remove_iterator != children.end()) {
            children.erase(child_to_remove_iterator,children.end());
        }
        out_degree--;        
    }
    
    // Print node details
    void print_node() const {
        std::cout << "Name: " << name << ", Date: " << date << ", Mutations: ";
        for (const auto& [segment, mut_set] : sample_mutations) {
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
    int r_index = 1; // index of reassortment node
    int h_index = 1; // index of non-reassortment hidden node
    std::string mutations_file_name;    
    std::vector<std::string> seg_names = {"PB1", "PB2", "PA", "HA", "NP", "NA", "M1", "NS1"};
    std::map<std::string, std::unique_ptr<Node>> nodes; // Map of nodes
    Node * root;
        

public:
    // Constructor
    Network(std::string mutations_file_name) {
        create_root(); // Replace with parser for root genome file
        read_mutations_from_file(mutations_file_name);
    }

    // Disable copy constructor & copy assignment (to enforce unique ownership)
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    // Default move constructor & move assignment
    Network(Network&&) = default;
    Network& operator=(Network&&) = default;

    // Function to add a node
    void add_node(const std::string& node_name, const std::string& date, const std::map<std::string, std::set<std::string>>& mutations) {
        if (nodes.find(node_name) != nodes.end()) {
            std::cerr << "Error: Node with name '" << node_name << "' already exists.\n";
            return;
        }
        nodes[node_name] = std::make_unique<Node>(node_name, date, mutations);
    }

    void create_root() {
        std::string root_name = "Root";
        std::string root_date = "1987-03-30"; // set using input data TMP_FLG
        std::map<std::string, std::set<std::string>> root_muts;
        for (std::string& seg_name: seg_names) {
            root_muts[seg_name] = std::set<std::string>();
        }
        add_node(root_name, root_date, root_muts);
        root = get_node(root_name);
    }

    void add_branch(Node * parent, Node * child, std::map<std::string, std::set<std::string>>& branch_mutations) {
        parent->add_child(child);
        child->set_parent(parent);
        child->set_branch_mutations(branch_mutations);
    }

    void remove_branch(Node * parent, Node * child) {
        child->remove_parent();
        parent->remove_child(child);
    }

    // Function to get a node (returns pointer or nullptr if not found)
    Node * get_node(const std::string& node_name) const {
        auto it = nodes.find(node_name);
        if (it != nodes.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // Function to remove a node
    void remove_node(const std::string& node_name) {
        if (nodes.erase(node_name) == 0) {
            std::cerr << "Error: Node with name '" << node_name << "' does not exist.\n";
        }
    }

    // Function to print all nodes in the network
    void print_network() const {
        std::cout << "Network Nodes:\n";
        for (const auto& pair : nodes) {
            pair.second->print_node();
        }
        std::cout << "Network Edges:\n";
        for (const auto& pair: nodes) {
            if (pair.second->parent != nullptr) {
                std::cout << "Start: " << pair.second->parent->get_name() << " End: " << pair.second->get_name() << " Mutations: " ;
                for (const auto& segment_mut_pair : pair.second->branch_mutations) {
                    const std::string& segment = segment_mut_pair.first;
                    const std::set<std::string>& mut_set = segment_mut_pair.second;
                    std::cout << segment << ":[";
                    for (auto it = mut_set.begin(); it != mut_set.end(); ++it) {
                        std::cout << *it;
                        if (std::next(it) != mut_set.end()) std::cout << ", ";
                    }
                    std::cout << "] ";
                }
                std::cout << std::endl;
            }            
        }
    }

    void graft_at_root(const std::string& node_name, const std::string& date, std::map<std::string, std::set<std::string>>& mutations) {
        std::cout << "Grafting " << node_name;
        add_node(node_name, date, mutations);
        Node * node = get_node(node_name);
        int net_size = nodes.size();
        assert(net_size == 2);
        std::cout << " as child of root" << std::endl;
        add_branch(root, node, mutations);
    }

    void graft_sample(const std::string& node_name, const std::string& date, std::map<std::string, std::set<std::string>>& mutations) {
        add_node(node_name, date, mutations);
        Node * sample_node = get_node(node_name);
        Node * graft_node;
        std::map <std::string,std::tuple<Node *, std::set<std::string>, std::set<std::string>>> graft_info;
        std::cout << "Grafting " << node_name;
        std::map <Node*, std::vector<std::string>> reassortment_groups;
        for (std::string seg: seg_names) {
            graft_info[seg] = get_1_graft_node_2_uniq_muts_in_sample_3_conflicting_muts_in_opt_path_for_seg(sample_node,seg);
            graft_node = std::get<0>(graft_info[seg]);
            if (reassortment_groups.count(graft_node)) {
                reassortment_groups[graft_node].push_back(seg);
            } else {
                reassortment_groups[graft_node] = std::vector<std::string>({seg});
            }            
        }
        if (reassortment_groups.size() > 1) {
            std::cout << "Reassortment detected" << std::endl;
            exit(-1); // Implement functionality
        } else {            
            graft_node = std::get<0>(graft_info[seg_names[0]]);
            if (graft_node->name == "Root") {
                assert(graft_node->in_degree == 0);
                std::cout << " as child of root" << std::endl;
                add_branch(root, sample_node, mutations);
            } else {
                Node * parent_node = graft_node->parent;
                std::cout << " along branch from " << parent_node->name << " to " << graft_node->name << std::endl;
                std::map<std::string,std::set<std::string>> sample_branch_uniq_muts; 
                for (std::string seg: seg_names) {
                    sample_branch_uniq_muts[seg] = std::get<1>(graft_info[seg]);
                }
                std::map<std::string,std::set<std::string>> graft_branch_common_muts;
                std::map<std::string,std::set<std::string>> graft_branch_uniq_muts; 
                std::map<std::string,std::set<std::string>> graft_branch_all_muts = graft_node->getBranchMutations();
                for (std::string seg: seg_names) {
                    std::set<std::string> conflicting_muts_on_opt_path = std::get<2>(graft_info[seg]);
                    graft_branch_common_muts[seg] = std::set<std::string>();
                    graft_branch_uniq_muts[seg] = std::set<std::string>();
                    for (std::string mut: graft_branch_all_muts[seg]) {
                        if (conflicting_muts_on_opt_path.count(mut)) {
                            graft_branch_uniq_muts[seg].emplace(mut); // hidden_node to graft_node
                        } else {
                            graft_branch_common_muts[seg].emplace(mut); // parent_node to hidden_node
                        }
                    }
                }
                remove_branch(parent_node, graft_node);

                std::string H_name = "H_" + std::to_string(h_index);
                h_index ++;
                std::string H_date = "0000-00-30"; // set using input data TMP_FLG
                std::map <std::string, std::set<std::string>> H_muts;
                for (std::string& seg_name: seg_names) {
                    H_muts[seg_name] = std::set<std::string>();
                }
                add_node(H_name, H_date, H_muts);
                Node * hidden_node = get_node(H_name);

                add_branch(parent_node, hidden_node, graft_branch_common_muts);
                add_branch(hidden_node, graft_node, graft_branch_uniq_muts);
                add_branch(hidden_node, sample_node, sample_branch_uniq_muts);
            }
        }
    }

    // [TO DO] account for branches with no muts for seg
    // positions that are shared by uniq_muts_in_sample and conflicting_muts_in_opt_path may be reversions or sign of reassortment/recombination
    std::tuple<Node *, std::set<std::string>, std::set<std::string>> get_1_graft_node_2_uniq_muts_in_sample_3_conflicting_muts_in_opt_path_for_seg(Node * sample_node, std::string seg) {
        Node * graft_node; // (globally optimal) node to find
        Node * opt_node; // opt node among children of search node
        Node * search_node = root;
        opt_node = search_node;
        std::set<std::string> uniq_muts_in_sample = sample_node->sample_mutations[seg];
        std::set<std::string> matching_muts_child_branch;
        std::set<std::string> conflicting_muts_child_branch;
        std::set<std::string> matching_muts_opt_branch;
        std::set<std::string> conflicting_muts_opt_branch;
        std::set<std::string> conflicting_muts_opt_path;
        int opt_branch_matching_muts_count;
        int child_branch_matching_muts_count;
        bool continue_search = true;
        while (continue_search) {
            opt_branch_matching_muts_count = 0;
            matching_muts_opt_branch.clear();
            conflicting_muts_opt_branch.clear();
            for (Node * child: search_node->children) {
                child_branch_matching_muts_count = 0;
                matching_muts_child_branch.clear();
                conflicting_muts_child_branch.clear();
                for (std::string mut: child->branch_mutations[seg]) {
                    if (uniq_muts_in_sample.count(mut)) {
                        matching_muts_child_branch.emplace(mut);
                        child_branch_matching_muts_count ++;
                    } else {
                        conflicting_muts_child_branch.emplace(mut);
                    }
                }
                if (child_branch_matching_muts_count > opt_branch_matching_muts_count) {
                    opt_branch_matching_muts_count = child_branch_matching_muts_count;
                    matching_muts_opt_branch = matching_muts_child_branch;
                    conflicting_muts_opt_branch = conflicting_muts_child_branch;
                    opt_node = child;
                }
            }
            if (opt_branch_matching_muts_count == 0) {
                continue_search = false;
                graft_node = search_node;
            } else {
                search_node = opt_node;
                for (std::string mut: matching_muts_opt_branch) {
                    uniq_muts_in_sample.erase(mut);
                }
                for (std::string mut: conflicting_muts_opt_branch) {
                    conflicting_muts_opt_path.emplace(mut);
                }
            }
        }
        return(std::make_tuple(graft_node,uniq_muts_in_sample,conflicting_muts_opt_path));
    }

    // Function to load mutations from a CSV file
    void read_mutations_from_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return;
        }

        std::string line;
        bool firstLine = true;
        int lines_parsed = 0;
        unsigned long num_columns = 10;

        while (std::getline(file, line)) {
            lines_parsed +=1 ;
            std::istringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;

            // Tokenize line by ","
            while (std::getline(ss, token, ',')) {
                tokens.push_back(trim(token));  // Remove whitespace
            }

            if (firstLine) {                
                seg_names.clear();
                seg_names.assign(tokens.begin()+2, tokens.end()); // Store segment names from header (excluding Date and ID columns)                
                std::cout << "Segment names in header file are" << std::endl;
                for (std::string seg_name: seg_names) {
                    std::cout << seg_name << "\t";
                } std::cout << std::endl;
                firstLine = false;
                continue;
            }
            
            assert(tokens.size() == num_columns);
            // if (tokens.size() < 2) continue; // Skip invalid lines

            std::string date = trim(tokens[0]);
            std::string node_id = trim(tokens[1]);
            std::map<std::string, std::set<std::string>> mutations;

            // Parse mutations for each segment
            for (size_t i = 2; i < num_columns; i++) {
                std::istringstream mutationStream(tokens[i]);
                std::string mutation;
                std::set<std::string> mutationSet;
                // Tokenize mutations by ":"
                while (std::getline(mutationStream, mutation, ':')) {
                    mutationSet.insert(trim(mutation));  // Remove whitespace
                }
                if ((i - 2) < seg_names.size()) {
                    mutations[seg_names[i - 2]] = mutationSet; // Map mutations to correct segment
                }
            }
            if (lines_parsed == 2) {
                graft_at_root(node_id, date, mutations);
            } else {
                graft_sample(node_id, date, mutations);
            }        
        }
        file.close();
    }
};

#endif // NETWORK_H
