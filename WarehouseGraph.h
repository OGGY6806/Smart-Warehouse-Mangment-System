#ifndef WAREHOUSEGRAPH_H
#define WAREHOUSEGRAPH_H

#include <vector>
#include <unordered_map>
#include <list>
#include <limits>
#include <queue>
#include <algorithm>
#include <utility>
#include <iostream>

using namespace std;

// Represents the warehouse layout as a weighted graph
class WarehouseGraph {
private:
    // Adjacency list: Node -> list of {neighbor, weight}
    unordered_map<int, vector<pair<int, int>>> adj;

public:
    // Add a connection between two locations (undirected)
    void addEdge(int u, int v, int weight) {
        adj[u].push_back({v, weight});
        adj[v].push_back({u, weight}); 
    }

    // Dijkstra's Algorithm to find shortest path from startNode to endNode
    // Returns pair<TotalDistance, PathVector>
    pair<int, vector<int>> getShortestPath(int start, int end) {
        unordered_map<int, int> dist;
        unordered_map<int, int> parent;
        
        // Initialize distances to infinity
        for (auto& node : adj) {
            dist[node.first] = numeric_limits<int>::max();
        }
        // Ensure start/end are in the map even if they have no connections yet (safety check)
        if (dist.find(start) == dist.end()) dist[start] = numeric_limits<int>::max();
        if (dist.find(end) == dist.end()) dist[end] = numeric_limits<int>::max();

        dist[start] = 0;
        
        // Min-Priority Queue for Dijkstra: {distance, node}
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        pq.push({0, start});

        while (!pq.empty()) {
            int d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            // If we found a shorter path before, skip this stale entry
            if (d > dist[u]) continue;
            
            // Reached destination?
            if (u == end) break; 

            if (adj.find(u) != adj.end()) {
                for (auto& edge : adj[u]) {
                    int v = edge.first;
                    int weight = edge.second;
                    
                    // Initialize neighbor if not seen
                    if (dist.find(v) == dist.end()) dist[v] = numeric_limits<int>::max();
                    
                    if (dist[u] != numeric_limits<int>::max() && dist[u] + weight < dist[v]) {
                        dist[v] = dist[u] + weight;
                        parent[v] = u;
                        pq.push({dist[v], v});
                    }
                }
            }
        }

        // Reconstruct path
        vector<int> path;
        if (dist[end] == numeric_limits<int>::max()) {
            return {-1, path}; // Unreachable
        }

        int curr = end;
        while (curr != start) {
            path.push_back(curr);
            if (parent.find(curr) == parent.end()) break; // Should not happen if path exists
            curr = parent[curr];
        }
        path.push_back(start);
        reverse(path.begin(), path.end());
        
        return {dist[end], path};
    }

    void displayGraph() {
        cout << "\n--- Warehouse Layout (Graph Connections) ---\n";
        for (auto& node : adj) {
            cout << "Node " << node.first << " is connected to: ";
            for (auto& edge : node.second) {
                cout << "[Node " << edge.first << " | Dist: " << edge.second << "] "; 
            }
            cout << endl;
        }
    }
};

#endif
