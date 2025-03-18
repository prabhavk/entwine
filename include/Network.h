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

using namespace std;

// Helper function to trim whitespace from a string
inline string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == string::npos || last == string::npos) ? "" : str.substr(first, last - first + 1);
}


// Node Class
class Node {
    public:

    int in_degree = 0;
    int out_degree = 0;
    string name;
    string date;
    map<string, set<string>> sample_mutations; // Sample Mutations W.R.T Root Genome For Each Segment    
    map<string, set<string>> branch_mutations; // Branch Mutations For Each Segment
    Node * parent; // <-> This is usable for Non-Reassortment nodes where each segment has same parent
    vector<Node*> children;
    map<string,Node *> parent4seg; // <-> This is usable for Reassortment nodes where segments have different parents
    bool reassortment_node = false;
    // Constructor
    Node(const string& node_name, const string& node_date,
         const map<string, set<string>>& node_mutations)
        : name(node_name), date(node_date), sample_mutations(node_mutations) {}

    // Getters
    string get_name() const { return name; }
    string getDate() const { return date; }
    map<string, set<string>> getSampleMutations() const { return sample_mutations; }
    map<string, set<string>> getBranchMutations() const { return branch_mutations; }

    // Setters
    void set_branch_mutations(map<string, set<string>> & branch_mutations_to_set) {
        branch_mutations = branch_mutations_to_set;
    }

    void set_parent(Node* parent_to_set) {
        parent = parent_to_set;
        in_degree++;
    }

    void setParentForSegment(string seg_name, Node * parent_to_set) {
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
        vector<Node*>::iterator child_to_remove_iterator = remove(children.begin(),children.end(),child_to_remove);
        if (child_to_remove_iterator != children.end()) {
            children.erase(child_to_remove_iterator,children.end());
        }
        out_degree--;        
    }
    
    // Print node details
    void print_node() const {
        cout << "Name: " << name << ", Date: " << date << ", Mutations: ";
        for (const auto& [segment, mut_set] : sample_mutations) {
            cout << segment << ":[";
            for (auto it = mut_set.begin(); it != mut_set.end(); ++it) {
                cout << *it;
                if (next(it) != mut_set.end()) cout << ", ";
            }
            cout << "] ";
        }
        cout << endl;
    }
};

// Network Class
class Network {
private:
    int r_index = 1; // index of reassortment node
    int h_index = 1; // index of non-reassortment hidden node
    string mutations_file_name; 
    string network_file_name;   
    vector<string> seg_names = {"PB1", "PB2", "PA", "HA", "NP", "NA", "M1", "NS1"};
    map<string, unique_ptr<Node>> nodes; // Map of nodes
    Node * root;
        

public:
    // Constructor
    Network(string mutations_file_name, string network_file_name) : mutations_file_name(mutations_file_name), network_file_name(network_file_name) {
        create_root(); // Replace with parser for root genome file
        read_mutations_from_file();
        write_network();
        print_network();
    }

    // Disable copy constructor & copy assignment (to enforce unique ownership)
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    // Default move constructor & move assignment
    Network(Network&&) = default;
    Network& operator=(Network&&) = default;

    // Function to add a node
    void add_node(const string& node_name, const string& date, const map<string, set<string>>& mutations) {
        if (nodes.find(node_name) != nodes.end()) {
            cerr << "Error: Node with name '" << node_name << "' already exists.\n";
            return;
        }
        nodes[node_name] = make_unique<Node>(node_name, date, mutations);
    }

    void create_root() {
        string root_name = "Root";
        string root_date = "1987-03-30"; // set using input data TMP_FLG
        map<string, set<string>> root_muts;
        for (string& seg_name: seg_names) {
            root_muts[seg_name] = set<string>();
        }
        add_node(root_name, root_date, root_muts);
        root = get_node(root_name);
    }

    void add_branch(Node * parent, Node * child, map<string, set<string>>& branch_mutations) {
        parent->add_child(child);
        child->set_parent(parent);
        child->set_branch_mutations(branch_mutations);
    }

    void remove_branch(Node * parent, Node * child) {
        child->remove_parent();
        parent->remove_child(child);
    }

    // Function to get a node (returns pointer or nullptr if not found)
    Node * get_node(const string& node_name) const {
        auto it = nodes.find(node_name);
        if (it != nodes.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // Function to remove a node
    void remove_node(const string& node_name) {
        if (nodes.erase(node_name) == 0) {
            cerr << "Error: Node with name '" << node_name << "' does not exist.\n";
        }
    }

    // Function to write network to file
    void print_network() const {
        map <string,int> num_muts4seg = {};
        for (string seg_name: seg_names) {
            num_muts4seg[seg_name] = 0;
        }
        int tot_num_muts = 0;
        cout << "Network Nodes:\n";
        for (const auto& pair : nodes) {
            pair.second->print_node();
        }
        cout << "Network Edges:\n";
        for (const auto& pair: nodes) {
            if (pair.second->parent != nullptr) {
                cout << "Start: " << pair.second->parent->get_name() << " End: " << pair.second->get_name() << " Mutations: " ;
                for (const auto& segment_mut_pair : pair.second->branch_mutations) {
                    const string& segment = segment_mut_pair.first;
                    const set<string>& mut_set = segment_mut_pair.second;
                    num_muts4seg[segment] += mut_set.size();
                    tot_num_muts += mut_set.size();
                    cout << segment << ":[";
                    for (auto it = mut_set.begin(); it != mut_set.end(); ++it) {
                        cout << *it;
                        if (next(it) != mut_set.end()) cout << ", ";
                    }
                    cout << "] ";
                }
                cout << endl;
            }            
        }
        cout << "Total number of mutations is " << tot_num_muts << endl;
        for (string seg_name: seg_names) {
            cout << "Number of mutations in segment " << seg_name << " is " << num_muts4seg[seg_name] << endl;
        }
    }

    // Function to print all nodes in the network
    void write_network() const {
        map <string,int> num_muts4seg = {};
        for (string seg_name: seg_names) {
            num_muts4seg[seg_name] = 0;
        }
        int tot_num_muts = 0;
        ofstream out_file(network_file_name);
        if (!out_file) {
            cerr << "Error: Unable to open file " << network_file_name << " for writing.\n";
            return;
        }
    
        out_file << "Network Nodes:\n";
        for (const auto& pair : nodes) {
            out_file << pair.second->get_name() << "\n"; // Assuming get_name() exists for node names
        }
    
        out_file << "Network Edges:\n";
        for (const auto& pair : nodes) {
            if (pair.second->parent != nullptr) {
                out_file << "Start: " << pair.second->parent->get_name() 
                         << " End: " << pair.second->get_name() 
                         << " Mutations: ";
    
                for (const auto& segment_mut_pair : pair.second->branch_mutations) {
                    const string& segment = segment_mut_pair.first;
                    const set<string>& mut_set = segment_mut_pair.second;
                    num_muts4seg[segment] += mut_set.size();
                    tot_num_muts += mut_set.size();
                    out_file << segment << ":[";
                    for (auto it = mut_set.begin(); it != mut_set.end(); ++it) {
                        out_file << *it;
                        if (next(it) != mut_set.end()) out_file << ", ";
                    }
                    out_file << "] ";
                }
                out_file << "\n";
            }            
        }
        out_file << "Total number of mutations: " << tot_num_muts << endl;
        cout << "Total number of mutations is " << tot_num_muts << endl;
        for (string seg_name: seg_names) {
            cout << "Number of mutations in segment " << seg_name << " is " << num_muts4seg[seg_name] << endl;
        }
        out_file.close();
    }

    void graft_at_root(const string& node_name, const string& date, map<string, set<string>>& mutations) {
        // cout << "Grafting " << node_name;
        add_node(node_name, date, mutations);
        Node * node = get_node(node_name);
        int net_size = nodes.size();
        assert(net_size == 2);
        // cout << " as child of root" << endl;
        add_branch(root, node, mutations);
    }



    
    void graft_sample(const string& node_name, const string& date, map<string, set<string>>& mutations) {
        bool debug = true;
        add_node(node_name, date, mutations);
        Node * sample_node = get_node(node_name);
        Node * graft_node;
        map <string,tuple<Node *, set<string>, set<string>>> graft_info;               
        map <Node*, vector<string>> reassortment_groups;
        for (string seg: seg_names) {
            graft_info[seg] = get_1_graft_node_2_uniq_muts_in_sample_3_conflicting_muts_in_opt_path_for_seg(sample_node,seg);
            graft_node = get<0>(graft_info[seg]);
            if (debug) {
                cout << "Graft node is " << graft_node->name << " for segment " << seg << endl;
                cout << "Number of sample mutations in sample are " << sample_node->sample_mutations[seg].size() << endl;
                cout << "Number of unique mutations in sample are " << get<1>(graft_info[seg]).size() << endl;
                cout << "Number of conflicting mutations in optimal path are " << get<2>(graft_info[seg]).size() << endl;
                cout << "--------------------------------------------------------------------" << endl;
                
            }
            if (reassortment_groups.count(graft_node)) {
                reassortment_groups[graft_node].push_back(seg);
            } else {
                reassortment_groups[graft_node] = vector<string>({seg});
            }            
        }
        if (reassortment_groups.size() > 1) { // If there are two groups and graft node of one group is parent of another then it is not reassortment. 
                                              // If graft nodes are not parent-child then it is reassortment
            bool parent_child = false;
            if (reassortment_groups.size() == 2) {   
                cout << "Two groups" << endl;           
                for (auto group1: reassortment_groups) {
                    for (auto group2: reassortment_groups) {
                        if (group1.first->parent == group2.first || group2.first->parent == group1.first) {
                            cout << "parent-child" << endl;                            
                            if (group1.first != group2.first) {
                                parent_child = true;
                                cout << "parent-child" << endl;
                                // exit(-1);
                            }                            
                        }
                    }
                }                
            }
            if (parent_child) {
                cout << "parent-child" << endl;

            } else {
                if (debug) {
                    cout << "Reassortment detected" << endl;
                }            
                string R_name = "R_" + to_string(r_index);
                r_index ++;
                string R_date = "2025-03-14";
                map <string, set<string>> R_muts;
                for (string& seg_name: seg_names) {
                    R_muts[seg_name] = set<string>();
                }
                add_node(R_name, R_date, R_muts);
                Node * R_node = get_node(R_name);
                R_node->reassortment_node = true;
                // R_node->set_branch_mutations(R_muts);
                for (auto graft_node_seg_list_pair: reassortment_groups) {
                    Node * graft_node = graft_node_seg_list_pair.first;
                    vector<string> seg_list = graft_node_seg_list_pair.second; 
                    // cout << "Graft node is " << graft_node->name << " for segments "; for (string seg: seg_list) {cout << seg << " ";} cout << endl; 
                    Node * parent_node = graft_node->parent;
                    string H_name = "H_" + to_string(h_index) + "_" + R_name;
                    h_index++;
                    string H_date = "";
                    map <string, set<string>> H_muts;
                    for (string& seg_name: seg_names) {
                        H_muts[seg_name] = set<string>();
                    }
                    add_node(H_name, H_date, H_muts);
                    Node * H_node = get_node(H_name);
                    map<string,set<string>> graft_branch_uniq_muts;
                    map<string,set<string>> graft_branch_common_muts;
                    map<string,set<string>> reassortment_branch_seg_list;
                    map<string,set<string>> graft_branch_all_muts = graft_node->getBranchMutations();
                    for (string seg: seg_names) {
                        if (find(seg_list.begin(),seg_list.end(),seg) != seg_list.end()) {  // seg is in seg_list                     
                            R_node->setParentForSegment(seg,H_node);
                            set<string> conflicting_muts_on_opt_path = get<2>(graft_info[seg]);
                            for (string mut: graft_branch_all_muts[seg]) {
                                if (conflicting_muts_on_opt_path.count(mut)) {
                                    graft_branch_uniq_muts[seg].emplace(mut);
                                } else {
                                    graft_branch_common_muts[seg].emplace(mut);
                                }
                            }
                        } else {  // seg is not in seg_list
                            graft_branch_common_muts[seg] = set<string>();
                            graft_branch_uniq_muts[seg] = graft_branch_all_muts[seg];
                        }                    
                    }                
                    remove_branch(parent_node, graft_node);
                    add_branch(parent_node, H_node, graft_branch_common_muts);
                    add_branch(H_node, graft_node, graft_branch_uniq_muts);
                    add_branch(H_node, R_node, R_muts);
                    map<string,set<string>> sample_branch_uniq_muts; 
                    for (string seg: seg_names) {
                        sample_branch_uniq_muts[seg] = get<1>(graft_info[seg]);
                    }
                    add_branch(R_node, sample_node, sample_branch_uniq_muts);
                }
            }                                                          
        } else {            
            graft_node = get<0>(graft_info[seg_names[0]]);
            if (graft_node->name == "Root") {
                assert(graft_node->in_degree == 0);
                cout << " as child of root" << endl;
                add_branch(root, sample_node, mutations);
            } else {
                Node * parent_node = graft_node->parent;
                cout << " along branch from " << parent_node->name << " to " << graft_node->name << endl;
                map<string,set<string>> sample_branch_uniq_muts; 
                for (string seg: seg_names) {
                    sample_branch_uniq_muts[seg] = get<1>(graft_info[seg]);
                }
                map<string,set<string>> graft_branch_common_muts;
                map<string,set<string>> graft_branch_uniq_muts; 
                map<string,set<string>> graft_branch_all_muts = graft_node->getBranchMutations();
                for (string seg: seg_names) {
                    set<string> conflicting_muts_on_opt_path = get<2>(graft_info[seg]);
                    graft_branch_common_muts[seg] = set<string>();
                    graft_branch_uniq_muts[seg] = set<string>();
                    for (string mut: graft_branch_all_muts[seg]) {
                        if (conflicting_muts_on_opt_path.count(mut)) {
                            graft_branch_uniq_muts[seg].emplace(mut); // hidden_node to graft_node
                        } else {
                            graft_branch_common_muts[seg].emplace(mut); // parent_node to hidden_node
                        }
                    }
                }
                remove_branch(parent_node, graft_node);

                string H_name = "H_" + to_string(h_index);
                h_index ++;
                string H_date = "0000-00-30"; // set using input data TMP_FLG
                map <string, set<string>> H_muts;
                for (string& seg_name: seg_names) {
                    H_muts[seg_name] = set<string>();
                }
                add_node(H_name, H_date, H_muts);
                Node * hidden_node = get_node(H_name);
                // cout << "Hidden node is " << hidden_node->name << endl;
                add_branch(parent_node, hidden_node, graft_branch_common_muts);
                add_branch(hidden_node, graft_node, graft_branch_uniq_muts);
                add_branch(hidden_node, sample_node, sample_branch_uniq_muts);
            }
        }
    }

    // positions that are shared by uniq_muts_in_sample and conflicting_muts_in_opt_path may be reversions or sign of reassortment/recombination
    tuple <Node *, set<string>, set<string>> get_1_graft_node_2_uniq_muts_in_sample_3_conflicting_muts_in_opt_path_for_seg(Node * sample_node, string seg) {
        bool debug = false;
        Node * graft_node; // (globally optimal) node to find
        Node * opt_node = root; // opt node among children of search node
        Node * search_node = root;
        // opt_node = search_node;
        set<string> uniq_muts_in_sample = sample_node->sample_mutations[seg];
        set<string> matching_muts_child_branch;
        set<string> conflicting_muts_child_branch;
        set<string> matching_muts_opt_branch;
        set<string> conflicting_muts_opt_branch;
        set<string> conflicting_muts_opt_path;
        bool no_muts_on_branch = false;
        int opt_branch_matching_muts_count;
        int child_branch_matching_muts_count;
        bool continue_search = true;
        int loop_count = 0;
        // cout << endl;
        while (continue_search) {     
            if (debug) {
                cout << "search node is " << search_node->name << " for segment " << seg << endl;
            }            
            loop_count ++;            
            opt_branch_matching_muts_count = 0;
            matching_muts_opt_branch.clear();
            conflicting_muts_opt_branch.clear();
            no_muts_on_branch = false;
            for (Node * child: search_node->children) {
                if (debug) {
                    cout << "child name is " << child->name << endl;
                }                
                child_branch_matching_muts_count = 0;
                matching_muts_child_branch.clear();
                conflicting_muts_child_branch.clear();
                child->branch_mutations[seg].size() == 0 ? no_muts_on_branch = true : no_muts_on_branch = false;
                // if (no_muts_on_branch == false) {
                //     child->branch_mutations[seg].size() == 0 ? no_muts_on_branch = true : no_muts_on_branch = false;
                // } else {
                //     assert(child->branch_mutations[seg].size() > 0); // no_muts_on_branch should be true only for one child branch, not more
                // }                
                for (string mut: child->branch_mutations[seg]) {
                    if (uniq_muts_in_sample.count(mut)) {
                        matching_muts_child_branch.emplace(mut);
                        child_branch_matching_muts_count ++;
                    } else {
                        conflicting_muts_child_branch.emplace(mut);
                    }
                }
                if (child_branch_matching_muts_count > opt_branch_matching_muts_count || no_muts_on_branch) {
                    opt_branch_matching_muts_count = child_branch_matching_muts_count;
                    matching_muts_opt_branch = matching_muts_child_branch;
                    conflicting_muts_opt_branch = conflicting_muts_child_branch;
                    opt_node = child;
                }
            }
            if (opt_branch_matching_muts_count > 0 || no_muts_on_branch) {
                // cout << "search node is " << search_node->in_degree << " for segment " << seg << endl;
                search_node = opt_node;
                assert(search_node != 0);
                // cout << "opt node is " << opt_node->name << " for segment " << seg << endl;
                for (string mut: matching_muts_opt_branch) {
                    uniq_muts_in_sample.erase(mut);
                }
                for (string mut: conflicting_muts_opt_branch) {
                    conflicting_muts_opt_path.emplace(mut);
                }                
            } else {
                continue_search = false;
                graft_node = search_node;
            }
            if (loop_count > 100) {                
                cout << "Network size is " << nodes.size() << endl;
                cout << "Attempting to graft " << sample_node->name << " for segment " << seg << endl;
                cout << "Search node is " << search_node->name << " for segment " << seg << endl;
                cout << "Optimum branch matching muts count is " << opt_branch_matching_muts_count << endl;
                cerr << "Error: Loop count exceeded 100" << endl;
                exit(-1);
            }
        }
        return(make_tuple(graft_node,uniq_muts_in_sample,conflicting_muts_opt_path));
    }

    // Function to load mutations from a CSV file
    void read_mutations_from_file() {
        ifstream file(mutations_file_name);
        if (!file) {
            cerr << "Error: Could not open file " << mutations_file_name << endl;
            return;
        }

        string line;
        bool firstLine = true;
        int lines_parsed = 0;
        unsigned long num_columns = 10;

        while (getline(file, line)) {
            lines_parsed +=1 ;
            cout << "Parsing line " << lines_parsed << endl;
            istringstream ss(line);
            string token;
            vector<string> tokens;

            // Tokenize line by ","
            while (getline(ss, token, ',')) {
                tokens.push_back(trim(token));  // Remove whitespace
            }

            if (firstLine) {                
                seg_names.clear();
                seg_names.assign(tokens.begin()+2, tokens.end()); // Store segment names from header (excluding Date and ID columns)                
                // cout << "Segment names in mutation file are" << endl;
                for (string seg_name: seg_names) {
                    cout << seg_name << "\t";
                } cout << endl;
                firstLine = false;
                continue;
            }
            
            assert(tokens.size() == num_columns);
            // if (tokens.size() < 2) continue; // Skip invalid lines

            string date = trim(tokens[0]);
            string node_id = trim(tokens[1]);
            map<string, set<string>> mutations;

            // Parse mutations for each segment
            for (size_t i = 2; i < num_columns; i++) {
                istringstream mutationStream(tokens[i]);
                string mutation;
                set<string> mutationSet;
                // Tokenize mutations by ":"
                while (getline(mutationStream, mutation, ':')) {
                    mutationSet.insert(trim(mutation));  // Remove whitespace
                }
                if ((i - 2) < seg_names.size()) {
                    mutations[seg_names[i - 2]] = mutationSet; // Map mutations to correct segment
                }
            }
            if (lines_parsed == 2) {
                cout << "Grafting " << node_id << endl;
                graft_at_root(node_id, date, mutations);
            } else {
                cout << "Grafting " << node_id << endl;
                graft_sample(node_id, date, mutations);
            }        
        }
        file.close();
    }
};

#endif // NETWORK_H
