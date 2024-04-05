#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <queue>
#include <algorithm>
#include "NNUtil.hpp"

using namespace std;


struct Edge {
    int id;
    int vertices[2];
    float weight;
    int sharedIndices;

    friend bool operator<(const Edge& l, const Edge& r) {
        if (l.weight < r.weight) return true;
        return false;
    }

    friend bool operator>(const Edge& l, const Edge& r) {
        return r < l;
    }

    friend bool operator<=(const Edge& l, const Edge& r) {
        return !(l > r);
    }

    friend bool operator>=(const Edge& l, const Edge& r) {
        return !(l < r);
    }

    friend bool operator==(const Edge& l, const Edge& r) {
        return l.vertices[0] == r.vertices[0] 
                && l.vertices[1] == r.vertices[1] 
                && l.weight == r.weight 
                && l.sharedIndices == r.sharedIndices;
    }

    friend bool operator!=(const Edge& l, const Edge& r) {
        return !(l == r);
    }
};

std::string edgeToString(Edge& e) {
    return "id: " + std::to_string(e.id) + ", weight: " + std::to_string(e.weight) + ", vertices: [" + std::to_string(e.vertices[0]) + ", " + std::to_string(e.vertices[1]) + "], sharedIndices: " + std::to_string(e.sharedIndices);
}

struct Vertex {
    int idx;
    std::vector<int> edges;
    torch::Tensor features;
};


class Graph {
    // Private members


public:
    std::map<int, Vertex> vertices;
    std::map<int, Edge> edges;
    const int maxVertexId;

    std::queue<int> workingQueue;
    torch::jit::script::Module model;

    vTensorCombinator tensorCombiner;

    Graph(std::map<int, Vertex> vertices, std::map<int, Edge> edges, std::queue<int> workingQueue, 
        torch::jit::script::Module model, vTensorCombinator tensorCombiner, const int maxVertexId)
            : vertices(vertices), edges(edges), workingQueue(workingQueue), model(model), tensorCombiner(tensorCombiner), maxVertexId(maxVertexId)
        {
            if (!this->workingQueue.empty())
                this->updateEdges();

        }    

    int generateEdgeIdx(Vertex v1, Vertex v2) {
        return v1.idx < v2.idx ? v1.idx * this->maxVertexId + v2.idx : v2.idx * this->maxVertexId + v1.idx;
    }

    void contractEdge(int edgeIdx) {
        Edge edge = this->edges[edgeIdx];
        Vertex left = this->vertices[edge.vertices[0]];
        Vertex right = this->vertices[edge.vertices[1]];

        // Prepare new vertex
        right.features = this->tensorCombiner(left.features, right.features, edge.weight, edge.sharedIndices);

        for (int i = 0; i < left.edges.size(); i++) {
            int new_edge_name = this->generateEdgeIdx(left, right);
            if (left.edges[i] != edge.id || this->edges.find(new_edge_name) == this->edges.end()) {
                right.edges.push_back(left.edges[i]);
            } else if (left.edges[i] != edge.id) {
                // Combine the two edges going to same vertices
                this->edges[new_edge_name].sharedIndices += this->edges[left.edges[i]].sharedIndices;
            }
        }

        // Remedy affected edges
        
        for (int i = 0; i < right.edges.size(); i++) {
            for (int j = 0; j < 2; j++) {
                if (this->edges[right.edges[i]].vertices[0] == edge.vertices[j])
                    this->edges[right.edges[i]].vertices[0] = edge.vertices[1];
                else if (this->edges[right.edges[i]].vertices[1] == edge.vertices[j])
                    this->edges[right.edges[i]].vertices[1] = edge.vertices[1];
            }
        }

        // Schedule edges for recomputation
        for (int i = 0; i < right.edges.size(); i++) {
            this->workingQueue.push(right.edges[i]);
        }

        printf("%d edges before\n", this->edges.size());
        // Remove contracted edge
        const auto it = this->edges.find(edgeIdx);
        if (it != this->edges.end())
            this->edges.erase(it);
        printf("%d edges after\n", this->edges.size());

        this->updateEdges();
    }

    void updateEdges() {
        while (!this->workingQueue.empty()) {
            int elemIdx = this->workingQueue.front();
            Edge edge = this->edges[elemIdx];
            float new_pred = applyModel(this->model, this->vertices[edge.vertices[0]].features, this->vertices[edge.vertices[1]].features, edge.sharedIndices);

            this->edges[elemIdx].weight = new_pred;

            this->workingQueue.pop();
        }
    }
};




Graph initialiseGraph(std::map<int, gate> gate_set, std::map<int, std::vector<dd::Index>> index_set, std::vector<std::tuple<int, int>> pythonEdges, torch::jit::script::Module model) {
    std::map<int, Edge> edges = {};
    std::map<int, Vertex> vertices = {};
    std::queue<int> workingQueue = {};

    int maxVertexId = 1;
    for (int i = 0; i < pythonEdges.size(); i++) {
        if (std::get<0>(pythonEdges[i]) > maxVertexId)
            maxVertexId = std::get<0>(pythonEdges[i]);        
        if (std::get<1>(pythonEdges[i]) > maxVertexId)
            maxVertexId = std::get<1>(pythonEdges[i]);
    }
    maxVertexId++;

    for (int i = 0; i < pythonEdges.size(); i++) {
        Edge new_edge;
        new_edge.id = std::get<0>(pythonEdges[i]) * maxVertexId + std::get<1>(pythonEdges[i]);
        new_edge.vertices[0] = std::get<0>(pythonEdges[i]);
        new_edge.vertices[1] = std::get<1>(pythonEdges[i]);
        
        int leftIdx = plan_offset[std::get<0>(pythonEdges[i])];
        int rightIdx = plan_offset[std::get<1>(pythonEdges[i])];

        // torch::Tensor leftTensor = makeTDDTensorFromValues(gate_set[leftIdx].name, index_set[leftIdx].size());
        // torch::Tensor rightTensor = makeTDDTensorFromValues(gate_set[rightIdx].name, index_set[rightIdx].size());

        int sharedIndices = 0;
        for (int j = 0; j < index_set[leftIdx].size(); j++) {
            for (int k = 0; k < index_set[rightIdx].size(); k++) {
                if (index_set[leftIdx][j] == index_set[rightIdx][k])
                    sharedIndices++;
            }
        }
        new_edge.sharedIndices = sharedIndices;

        edges[new_edge.id] = new_edge;
        workingQueue.push(new_edge.id);

        int idx = std::get<0>(pythonEdges[i]);
        if (vertices.find(idx) != vertices.end()) {
            vertices[idx].edges.push_back(new_edge.id);
        } else {
            Vertex vertex;
            int vertIdx = plan_offset[idx];
            vertex.idx = vertIdx;
            vertex.features = makeTDDTensorFromValues(gate_set[vertIdx].name, index_set[vertIdx].size());
            vertex.edges = { new_edge.id };
            vertices[idx] = vertex;
        }

        idx = std::get<1>(pythonEdges[i]);
        if (vertices.find(idx) != vertices.end()) {
            vertices[idx].edges.push_back(new_edge.id);
        } else {
            Vertex vertex;
            int vertIdx = plan_offset[idx];
            vertex.features = makeTDDTensorFromValues(gate_set[vertIdx].name, index_set[vertIdx].size());
            vertex.edges = { new_edge.id };
            vertices[idx] = vertex;
        }

    }


    return Graph(vertices, edges, workingQueue, model, format16Combiner, maxVertexId);
}

std::vector<std::tuple<int, int>> GreedyPlan(std::map<int, gate> gate_set, std::map<int, std::vector<dd::Index>> index_set, 
                                                std::vector<std::tuple<int, int>> pythonEdges, torch::jit::script::Module model) {
    Graph g = initialiseGraph(gate_set, index_set, pythonEdges, model);
    printf("Successfully made graph\n");

    std::vector<std::tuple<int, int>> plan = {};
    int edgeCount = g.edges.size();
    for (int i = 0; i < edgeCount; i++) {
        printf("%d edges initially\n", g.edges.size());
        int min_edge = -1;
        for (auto it = g.edges.begin(); it != g.edges.end(); ++it) {
            //printf("\t%d: %s\n", key, edgeToString(val));
            int key = it->first;
            Edge& val = it->second;
            printf("\t%d: %s\n", key, edgeToString(val).c_str());
            if (min_edge == -1 || g.edges[min_edge] > val) {
                min_edge = key;
            }
        }
        printf("Step %d: min key is = %d\n", i, min_edge);
        
        plan.push_back({g.edges[min_edge].vertices[0], g.edges[min_edge].vertices[1]});

        printf("%d edges before\n", g.edges.size());
        g.contractEdge(min_edge);
        printf("%d edges after\n", g.edges.size());
        printf("Step %d: done contracting\n", i);
    }

    return plan;
}