#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <cassert>
#include <limits>
#include <cmath>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace chrono;

using Graph = map<int, vector<pair<int, double>>>;
using Coordinates = map<int, pair<double, double>>;

void parseGraph(const string& filename, Graph& graph, Coordinates& coordinates) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    map<pair<double, double>, int> coordinateToId;
    int nodeId = 0;

    auto getId = [&](double lon, double lat) {
        pair<double, double> coord = {lon, lat};
        if (!coordinateToId.count(coord)) {
            coordinateToId[coord] = nodeId;
            coordinates[nodeId] = coord;
            return nodeId++;
        }
        return coordinateToId[coord];
    };

    string line;
    while (getline(file, line)) {
        size_t colon = line.find(":");
        if (colon == string::npos) continue;

        string parentNode = line.substr(0, colon);
        string children = line.substr(colon + 1);

        double lon1, lat1;
        sscanf(parentNode.c_str(), "%lf,%lf", &lon1, &lat1);
        int parentId = getId(lon1, lat1);

        stringstream ss(children);
        string child;
        while (getline(ss, child, ';')) {
            double lon2, lat2, weight;
            if (sscanf(child.c_str(), "%lf,%lf,%lf", &lon2, &lat2, &weight) == 3) {
                int childId = getId(lon2, lat2);
                graph[parentId].emplace_back(childId, weight);
                graph[childId].emplace_back(parentId, weight);
            }
        }
    }
}

int findNearestNode(const Coordinates& coordinates, double lon, double lat) {
    int nearestNode = -1;
    double minDistance = numeric_limits<double>::infinity();

    for (const auto& [node, coord] : coordinates) {
        double distance = sqrt(pow(coord.first - lon, 2) + pow(coord.second - lat, 2));
        if (distance < minDistance) {
            minDistance = distance;
            nearestNode = node;
        }
    }
    return nearestNode;
}

double printPath(const map<int, int>& parent, int start, int goal, const Graph& graph) {
    vector<int> path;
    double weight = 0;
    for (int cur = goal; cur != start; cur = parent.at(cur)) {
        path.push_back(cur);
        for (const auto& [neighbor, w] : graph.at(parent.at(cur)))
            if (neighbor == cur) weight += w;
    }
    path.push_back(start);

    reverse(path.begin(), path.end());
    return weight;
}

double DFS(const Graph& graph, int start, int goal) {
    auto start_time = high_resolution_clock::now();

    stack<int> s;
    set<int> visited;
    map<int, int> parent;
    s.push(start);

    while (!s.empty()) {
        int current = s.top();
        s.pop();

        if (visited.count(current)) continue;
        visited.insert(current);

        if (current == goal) {
            auto end_time = high_resolution_clock::now();
            double weight = printPath(parent, start, goal, graph);
            cout << "DFS Time: " << duration_cast<milliseconds>(end_time - start_time).count() << " ms\n";
            cout << "Total weight: " << weight << endl;
            return weight;
        }

        for (const auto& [neighbor, _] : graph.at(current)) {
            if (!visited.count(neighbor)) {
                s.push(neighbor);
                parent[neighbor] = current;
            }
        }
    }
    cout << "DFS: No path found.\n";
    return -1;
}

double BFS(const Graph& graph, int start, int goal) {
    auto start_time = high_resolution_clock::now();

    queue<int> q;
    set<int> visited;
    map<int, int> parent;
    q.push(start);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        if (visited.count(current)) continue;
        visited.insert(current);

        if (current == goal) {
            auto end_time = high_resolution_clock::now();
            double weight = printPath(parent, start, goal, graph);
            cout << "BFS Time: " << duration_cast<milliseconds>(end_time - start_time).count() << " ms\n";
            cout << "Total weight: " << weight << endl;
            return weight;
        }

        for (const auto& [neighbor, _] : graph.at(current)) {
            if (!visited.count(neighbor)) {
                q.push(neighbor);
                parent[neighbor] = current;
            }
        }
    }
    cout << "BFS: No path found.\n";
    return -1;
}

double Dijkstra(const Graph& graph, int start, int goal) {
    auto start_time = high_resolution_clock::now();

    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
    map<int, double> distances;
    map<int, int> parent;

    for (const auto& node : graph) distances[node.first] = numeric_limits<double>::infinity();
    distances[start] = 0;

    pq.emplace(0, start);

    while (!pq.empty()) {
        auto [current_distance, current_node] = pq.top();
        pq.pop();

        if (current_node == goal) {
            auto end_time = high_resolution_clock::now();
            double weight = printPath(parent, start, goal, graph);
            cout << "Dijkstra Time: " << duration_cast<milliseconds>(end_time - start_time).count() << " ms\n";
            cout << "Total weight: " << weight << endl;
            return weight;
        }

        for (const auto& [neighbor, weight] : graph.at(current_node)) {
            double new_distance = current_distance + weight;

            if (new_distance < distances[neighbor]) {
                distances[neighbor] = new_distance;
                parent[neighbor] = current_node;
                pq.emplace(new_distance, neighbor);
            }
        }
    }
    cout << "Dijkstra: No path found.\n";
    return -1;
}

double AStar(const Graph& graph, int start, int goal, const Coordinates& coordinates) {
    auto start_time = high_resolution_clock::now();

    auto heuristic = [&](int node) {
        auto [lon1, lat1] = coordinates.at(node);
        auto [lon2, lat2] = coordinates.at(goal);
        return sqrt((lon1 - lon2) * (lon1 - lon2) + (lat1 - lat2) * (lat1 - lat2));
    };

    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
    map<int, double> g_costs;
    map<int, double> f_costs;
    map<int, int> parent;

    for (const auto& node : graph) {
        g_costs[node.first] = numeric_limits<double>::infinity();
        f_costs[node.first] = numeric_limits<double>::infinity();
    }
    g_costs[start] = 0;
    f_costs[start] = heuristic(start);

    pq.emplace(f_costs[start], start);

    while (!pq.empty()) {
        auto [current_f_cost, current_node] = pq.top();
        pq.pop();

        if (current_node == goal) {
            auto end_time = high_resolution_clock::now();
            double weight = printPath(parent, start, goal, graph);
            cout << "A* Time: " << duration_cast<milliseconds>(end_time - start_time).count() << " ms\n";
            cout << "Total weight: " << weight << endl;
            return weight;
        }

        for (const auto& [neighbor, weight] : graph.at(current_node)) {
            double tentative_g_cost = g_costs[current_node] + weight;

            if (tentative_g_cost < g_costs[neighbor]) {
                g_costs[neighbor] = tentative_g_cost;
                f_costs[neighbor] = tentative_g_cost + heuristic(neighbor);
                parent[neighbor] = current_node;
                pq.emplace(f_costs[neighbor], neighbor);
            }
        }
    }
    cout << "A*: No path found.\n";
    return -1;
}

void runTest(const string& filename, double lon1, double lat1, double lon2, double lat2) {
    Graph graph;
    Coordinates coordinates;
    parseGraph("/Users/mir/CLionProjects/lab-8/spb_graph.txt", graph, coordinates);

    int start = findNearestNode(coordinates, lon1, lat1);
    int goal = findNearestNode(coordinates, lon2, lat2);

    cout << "\nRunning DFS from (" << lon1 << ", " << lat1 << ") to (" << lon2 << ", " << lat2 << ")\n";
    assert(DFS(graph, start, goal) >= 0);

    cout << "\nRunning BFS from (" << lon1 << ", " << lat1 << ") to (" << lon2 << ", " << lat2 << ")\n";
    assert(BFS(graph, start, goal) >= 0);

    cout << "\nRunning Dijkstra from (" << lon1 << ", " << lat1 << ") to (" << lon2 << ", " << lat2 << ")\n";
    assert(Dijkstra(graph, start, goal) >= 0);

    cout << "\nRunning A* from (" << lon1 << ", " << lat1 << ") to (" << lon2 << ", " << lat2 << ")\n";
    assert(AStar(graph, start, goal, coordinates) >= 0);
}

int main() {
    cout << "All tests passed.\n";
    runTest("/Users/mir/CLionProjects/lab-8/test_graph.txt", 30.500014, 59.936521, 30.310011, 59.956363);

    return 0;
}
