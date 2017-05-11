#ifndef TRIANGLES_AUX_H
#define TRIANGLES_AUX_H

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include "constants.h"

using namespace std;


typedef struct {
    int from;
    int to;
} Edge;

typedef struct {
    int id;
    vector<int> neighbors;
} Node;


vector<string> split(string str, const char *delimiter);

int node_owner(int boundaries[], int size, int node);

long get_file_size(string fileName);

Edge parse_line(string line, const char *delimiter);

map<int, Node> parse_file(string filename, const char *delimiter);

int *partition_file(string filename, const char *delimiter, int chunks);

int *serializeNode(Node node);

Node deserializeNode(int buffer[], int size);

int intersectionCount(vector<int> a, vector<int> b);

int surrogateCount(map<int, Node> nodes, Node node);

#endif //TRIANGLES_AUX_H
