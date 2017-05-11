#include "auxiliary.h"


vector<string> split(string str, const char *delimiter) {

    vector<string> parts;
    stringstream ss(str); // Turn the string into a stream.
    string tok;

    while (getline(ss, tok, *delimiter)) {
        parts.push_back(tok);
    }

    return parts;
}

long get_file_size(string fileName) {
    ifstream file(fileName, ios::binary | ios::ate);
    long size = file.tellg();
    file.close();
    return size;
}

/**
 * parses a line and returns the edge
 */
Edge parse_line(string line, const char *delimeter) {
    vector<string> sep = split(line, delimeter);
    Edge edge;
    edge.from = std::stoi(sep[0]);
    edge.to = std::stoi(sep[1]);
    return edge;
}

/**
 * returns the number of elements that exist on both vectors
 */
int intersectionCount(vector<int> nodeAdj1, vector<int> nodeAdj2) {
    int sum = 0;
    for (int i = 0; i < nodeAdj1.size(); ++i) {
        for (int j = 0; j < nodeAdj2.size(); ++j) {
            if (nodeAdj1[i] == nodeAdj2[j])
				sum += 1;
        }
    }
//	unordered_set<int> s(a.begin(), a.end());
//	sum = count_if(b.begin(), b.end(), [&](int k) {return s.find(k) != s.end();});

    return sum;
}

/**
 * parses the a file with edges
 * and returns a map of nodes to neighbors
 */
map<int, Node> parse_file(string filename, const char *delimiter) {
    string line;

    ifstream file(filename);
    map<int, Node> nodes;
    file.seekg(0, file.beg);

    if (file.is_open()) {
	/*	for(auto const &node: nodes) {
			for(int i=0;i<node.neighbors.size();i++) {
				if(nodes[node.neighbours[i]].find(node) != nodes[node.neighbours[i]].end())
					nodes[node.neighbours[i]].remove(node);
			}
		}*/
        while (getline(file, line) && line.at(0) != '#') {
            if (line.at(0) != '#') { // don't read comments
                Edge edge = parse_line(line, delimiter);
                nodes[edge.from].id = edge.from;
                nodes[edge.from].neighbors.push_back(edge.to);
            }
        }
        file.close();
        remove(filename.c_str());

    }
    else {
        cout << "Unable to open file";
    }

    return nodes;
}

/**
 * The input file is divided into P chunks.
 * If a chunk boundary falls in the middle of an adjacency list,
 * the boundary is shifted so that the entire adjacency list
 * is in only one chunk.
 */
int *partition_file(string filename, const char *delimiter, int chunks) {

    string line;
    ifstream file(filename);
    long file_size = get_file_size(filename);
    long chunk_size = file_size / chunks;
    long boundary_split = chunk_size;
    bool toSplit = false;
    int *boundaries = new int[chunks];
    Edge last, curr_node;
    int current = 0;

    ofstream temp_file;
    temp_file.open(CHUNK_PREFIX + to_string(current) + ".txt");
    cout << "\t-splitting chunk " << current << "..." << endl;
    if (file.is_open()) {
        while (getline(file, line)) {

            if (line.at(0) != '#') { // don't read comments

                curr_node = parse_line(line, delimiter);

                if (toSplit) {

                    //check if we are in the middle of an adjacency list
                    if (last.a != curr_node.a) {
                        //close the current open file
                        temp_file.close();
                        boundaries[current] = last.a;

                        current++;
                        toSplit = false;

                        //open a new file to right the next chunk
                        temp_file.open(CHUNK_PREFIX + to_string(current) + ".txt");
                        cout << "\t-splitting chunk " << current << "..." << endl;
                    }

                } else {

                    //check if the file pointer has reached the next boundary
                    //if it has we can't split right now, we have to check
                    //if we are in the middle of an adjacency list
                    if (file.tellg() > boundary_split) {
                        last = parse_line(line, delimiter);
                        toSplit = true;
                        boundary_split += chunk_size;
                    }

                }

                temp_file << line << endl;
            }
        }

        file.close();
        temp_file.close();
        boundaries[current] = curr_node.a;
    }
    else {
        cout << "Unable to open file";
    }

    return boundaries;

}

/**
 * Node data struct to an integer buffer
 * Format: [<node_id> {list of neighbors}]
 */
int *serializeNode(Node node) {
    //allocate buffer array
    int *s = new int[node.neighbors.size() + 1];
    //set the first element = Node.id
    s[0] = node.id;
    //set the rest elements to be the vector elements
	for (int i = 0; i < node.neighbors.size(); ++i) {
        s[i + 1] = node.neighbors[i];
    }
    return s;
}


/**
 * Message buffer of integers back to a Node data struct
 */
Node deserializeNode(int buffer[], int size) {
    Node node;
    //get the Node.id
    node.id = buffer[0];
    //get the vector elements
    node.neighbors.reserve((unsigned long) (size - 1));
    for (int i = 1; i < size; ++i) {
        node.neighbors.push_back(buffer[i]);
    }
    return node;
}

/* Function to map node to process
 */
int node_owner(int boundaries[], int size, int node) {
    for (int i = 0; i < size; i++) {
        if (node <= boundaries[i]) return i;
    }
    return 0;
}

/**
 * The function for computing the surrogateCount
 * that is described in the paper's algorithm (Fig. 2)
 */
int surrogateCount(map<int, Node> nodes, Node node) {

    int sum = 0;
    for (int i = 0; i < node.neighbors.size(); ++i) {
        int id = node.neighbors[i];

        if (nodes.find(id) != nodes.end()) {
            sum += intersectionCount(nodes[id].neighbors, node.neighbors);
        }
    }

    return sum;
}
