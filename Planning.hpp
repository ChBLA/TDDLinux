#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <queue>
#include <algorithm>
#include <stdio.h>
#include "NNUtil.hpp"
#include "dd/Package.hpp"
#include "dd/Export.hpp"
#include"time.h"
#include <math.h>
#include <chrono>
#include <ctime>
#include <sys/time.h>

using namespace std;

const bool debug_planning = false;

struct gate {
	std::string name;
	short int qubits[2];
	//std::vector<dd::Index> index_set;
};
std::map<int, int> plan_offset;

struct Edge {
    int id;
    int vertices[2];
    float weight;
    int sharedIndices;
    float time;

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

    Edge copy() {
        Edge copy;
        copy.id = id;
        copy.vertices[0] = vertices[0];
        copy.vertices[1] = vertices[1];
        copy.weight = weight;
        copy.sharedIndices = sharedIndices;
        copy.time = time;
        return copy;
    }

};

std::string edgeToString(Edge& e) {
    return "id: " + std::to_string(e.id) + ", weight: " + std::to_string(e.weight) + ", vertices: [" + std::to_string(e.vertices[0]) + ", " + std::to_string(e.vertices[1]) + "], sharedIndices: " + std::to_string(e.sharedIndices);
}



struct Vertex {
    int idx;
    std::vector<int> edges;
    torch::Tensor features;

    Vertex copy() {
        Vertex copy;
        copy.edges = edges;
        copy.idx = idx;
        copy.features = features.clone().detach().set_requires_grad(false);
        return copy;
    }

};

std::string vertexToString(Vertex& v) {
    string edges = "[";
    for (int i = 0; i < v.edges.size(); i++)
        edges += std::to_string(v.edges[i]) + ", ";
    
    return "id: " + std::to_string(v.idx) + ", edges: " + edges + "]";
}

class Graph {
    // Private members


public:
    std::map<int, Vertex> vertices;
    std::map<int, Edge> edges;
    int maxVertexId;

    float timeForCurrentStep;
    std::vector<float> updateTime;
    std::vector<float> contractionTime;

    std::queue<int> workingQueue;
    torch::jit::script::Module model;

    vTensorCombinator tensorCombiner;

    Graph(std::map<int, Vertex> vertices, std::map<int, Edge> edges, std::queue<int> workingQueue, 
        torch::jit::script::Module model, vTensorCombinator tensorCombiner, const int maxVertexId)
            : vertices(vertices), edges(edges), workingQueue(workingQueue), model(model), tensorCombiner(tensorCombiner), maxVertexId(maxVertexId)
        {
            timeForCurrentStep = 0;
            updateTime = {};
            contractionTime = {};
            if (!this->workingQueue.empty())
                this->updateEdges();

        }    

    int generateEdgeIdx(Vertex v1, Vertex v2) {
        return v1.idx < v2.idx ? v1.idx * this->maxVertexId + v2.idx : v2.idx * this->maxVertexId + v1.idx;
    }

    void contractEdge(int edgeIdx, float actualVal=0.0f) {
        Edge edge = this->edges[edgeIdx];
        Vertex left = this->vertices[edge.vertices[0]];
        Vertex right = this->vertices[edge.vertices[1]];
        if (debug_planning)
            printf("Contracting edge: %d with vertices: %d %d\n", edgeIdx, edge.vertices[0], edge.vertices[1]);

        // Prepare new vertex
        right.features = this->tensorCombiner(left.features, right.features, actualVal != 0.0f ? actualVal : edge.weight, edge.sharedIndices);
        this->contractionTime.push_back(edge.time);
        this->updateTime.push_back(this->timeForCurrentStep);
        this->timeForCurrentStep = 0;

        // Delete edge from result
        right.edges.erase(std::remove(right.edges.begin(), right.edges.end(), edgeIdx), right.edges.end());
        
        if (debug_planning)
            printf("Right vertex just after deleting: %s\n", vertexToString(right).c_str());

        for (int i = 0; i < left.edges.size(); i++) {
            int nonLeftVertexIdx = this->edges[left.edges[i]].vertices[0] != left.idx ? this->edges[left.edges[i]].vertices[0] : this->edges[left.edges[i]].vertices[1];
            int new_edge_name = this->generateEdgeIdx(right, this->vertices[nonLeftVertexIdx]);
            
            if (debug_planning)
                printf("New edge name: %d, with maxVertexId=%d and nonLeftVertixId=%d and actual idx for both: %d %d\n", new_edge_name, this->maxVertexId, nonLeftVertexIdx, right.idx, this->vertices[nonLeftVertexIdx].idx);
            
            if (left.edges[i] != edge.id && this->edges.find(new_edge_name) == this->edges.end()) {
                if (debug_planning)
                    printf("New edge does not exist already\n");
                right.edges.push_back(new_edge_name);
                this->edges[new_edge_name] = this->edges[left.edges[i]];
                this->edges[new_edge_name].id = new_edge_name;

                for (int j = 0; j < this->vertices[nonLeftVertexIdx].edges.size(); j++) {
                    if (this->vertices[nonLeftVertexIdx].edges[j] == left.edges[i])
                        this->vertices[nonLeftVertexIdx].edges[j] = new_edge_name;
                }

            } else if (left.edges[i] != edge.id) {
                // Combine the two edges going to same vertices
                if (debug_planning)
                    printf("Edge exists already\n");
                this->edges[new_edge_name].sharedIndices += this->edges[left.edges[i]].sharedIndices;
                this->vertices[nonLeftVertexIdx].edges.erase(std::remove(this->vertices[nonLeftVertexIdx].edges.begin(), this->vertices[nonLeftVertexIdx].edges.end(), left.edges[i]), this->vertices[nonLeftVertexIdx].edges.end());
            }
            this->edges.erase(left.edges[i]);

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

        // Remove contracted edge
        const auto it = this->edges.find(edgeIdx);
        if (it != this->edges.end())
            this->edges.erase(it);

        this->vertices[right.idx] = right;
        this->vertices[left.idx] = {left.idx, {}, {}};
        this->updateEdges();
    }

    void updateEdges() {
        while (!this->workingQueue.empty()) {
            int elemIdx = this->workingQueue.front();
            if (debug_planning)
                printf("Updating edge: %d\n", elemIdx);
            
            Edge edge = this->edges[elemIdx];
            struct timeval start, end;
	        gettimeofday(&start, NULL);
            
            float new_pred = applyModel(this->model, this->vertices[edge.vertices[0]].features, this->vertices[edge.vertices[1]].features, edge.sharedIndices);

            gettimeofday(&end, NULL);
            edge.time = getMTime(start, end);

            this->edges[elemIdx].weight = new_pred;
            this->workingQueue.pop();
            this->timeForCurrentStep += edge.time;
        }
    }

    bool doneWithContraction() {
        return this->edges.size() < 1;
    }

    int edgeCount() {
        return this->edges.size();
    }

    Graph copy() {
        std::map<int, Vertex> copyVertices = {};
        for (auto it = this->vertices.begin(); it != this->vertices.end(); ++it) {
            int key = it->first;
            Vertex& val = it->second;
            copyVertices[key] = val.copy();
        }

        std::map<int, Edge> copyEdges = {};
        for (auto it = this->edges.begin(); it != this->edges.end(); ++it) {
            int key = it->first;
            Edge& val = it->second;
            copyEdges[key] = val.copy();
        }

        return Graph(copyVertices, copyEdges, workingQueue, model, format16Combiner, maxVertexId);
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
        if (edges.find(new_edge.id) != edges.end()) {
            edges[new_edge.id].sharedIndices++;
            continue;
        }
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
            vertex.idx = idx;
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
            vertex.idx = idx;
            vertex.features = makeTDDTensorFromValues(gate_set[vertIdx].name, index_set[vertIdx].size());
            vertex.edges = { new_edge.id };
            vertices[idx] = vertex;
        }

    }


    return Graph(vertices, edges, workingQueue, model, format16Combiner, maxVertexId);
}

std::tuple<int, int, float> GreedyPlanNextStep(Graph& g) {
    int min_edge = -1;
    for (auto it = g.edges.begin(); it != g.edges.end(); ++it) {
        int key = it->first;
        Edge& val = it->second;
        if (min_edge == -1 || g.edges[min_edge] > val) {
            min_edge = key;
        }
    }

    float weight = g.edges[min_edge].weight;
    std::tuple<int, int> step = {g.edges[min_edge].vertices[0], g.edges[min_edge].vertices[1]};
    g.contractEdge(min_edge);

    return {std::get<0>(step), std::get<1>(step), weight};
}

std::vector<std::tuple<int, int, float>> FastGreedyPlan(std::map<int, gate> gate_set, std::map<int, std::vector<dd::Index>> index_set, 
                                                std::vector<std::tuple<int, int>> pythonEdges, torch::jit::script::Module model) {
    Graph g = initialiseGraph(gate_set, index_set, pythonEdges, model);
    std::vector<std::tuple<int, int, float>> plan = {};
    int edgeCount = g.edges.size();

    for (int i = 0; i < edgeCount - 1; i++) {
        plan.push_back(GreedyPlanNextStep(g));
    }

    return plan;
}

std::vector<std::tuple<int, int>> GreedyPlan(std::map<int, gate> gate_set, std::map<int, std::vector<dd::Index>> index_set, 
                                                std::vector<std::tuple<int, int>> pythonEdges, torch::jit::script::Module model) {
    Graph g = initialiseGraph(gate_set, index_set, pythonEdges, model);
    printf("Successfully made graph\n");

    if (debug_planning) {
        printf("Vertices: \n");
        for (auto it = g.vertices.begin(); it != g.vertices.end(); ++it) {
            int key = it->first;
            Vertex& val = it->second;
            printf("\t%d: %s\n", key, vertexToString(val).c_str());
        }
    }

    std::vector<std::tuple<int, int>> plan = {};
    int edgeCount = g.edges.size();
    int steps = 0;
    while (!g.doneWithContraction()) {
        if (debug_planning)
            printf("%d edges initially\n", g.edges.size());
        
        int min_edge = -1;
        for (auto it = g.edges.begin(); it != g.edges.end(); ++it) {
            //printf("\t%d: %s\n", key, edgeToString(val));
            int key = it->first;
            Edge& val = it->second;
            if (debug_planning)
                printf("\t%d: %s\n", key, edgeToString(val).c_str());
            
            if (min_edge == -1 || g.edges[min_edge] > val) {
                min_edge = key;
            }
        }
        
        if (debug_planning) {
            printf("Step %d: min key is = %d\n", steps, min_edge);
            printf("%d edges before\n", g.edges.size());
        }

        plan.push_back({g.edges[min_edge].vertices[0], g.edges[min_edge].vertices[1]});

        g.contractEdge(min_edge);
        if (debug_planning) {
            printf("%d edges after\n", g.edges.size());
            printf("Step %d: done contracting\n", steps);
        }

        if (debug_planning) {
            printf("Vertices: \n");
            for (auto it = g.vertices.begin(); it != g.vertices.end(); ++it) {
                int key = it->first;
                Vertex& val = it->second;
                printf("\t%d: %s\n", key, vertexToString(val).c_str());
            }
        }

        steps++;
    }

    return plan;
}


void printCurrentPlan(std::vector<std::tuple<int,int,float>> plan, std::string name) {
    std::string res = name + ": ";
    for (int i = 0; i < plan.size(); i++) {
        res += "\t(" + std::to_string(std::get<0>(plan[i])) + ", " + std::to_string(std::get<1>(plan[i])) + ") = " + std::to_string(std::get<2>(plan[i])) + "   ";
    }
    res += "\n";

    printf(res.c_str());
}

bool OnlineRGreedy(Graph g, std::vector<std::tuple<int, int, float>>* planned, std::vector<std::tuple<int, int, float>>* performed) {
    Graph copyGraph = g.copy();
    Graph workingGraph = g.copy();
    int lastSeenLengthOfPerformed = 0;
    while (!workingGraph.doneWithContraction()) {
        if ((*performed).size() > lastSeenLengthOfPerformed) {
            auto performedSteps = std::vector<std::tuple<int,int,float>>((*performed).begin() + lastSeenLengthOfPerformed, (*performed).end());
            lastSeenLengthOfPerformed += performedSteps.size();
            for (int i = 0; i < performedSteps.size(); i++) {
                // Extract values from step
                int leftIdx = std::get<0>(performedSteps[i]);
                int rightIdx = std::get<1>(performedSteps[i]);
                float actualVal = std::get<2>(performedSteps[i]);
            
                // Contract edge in graph
                int edgeName = copyGraph.generateEdgeIdx(copyGraph.vertices[leftIdx], copyGraph.vertices[rightIdx]);
                copyGraph.contractEdge(edgeName, actualVal);
            }

            // A contraction has been performed, reset working graph
            workingGraph = copyGraph;
            copyGraph = workingGraph.copy();

            (*planned).resize(lastSeenLengthOfPerformed);
        }

        (*planned).push_back(GreedyPlanNextStep(workingGraph));

        // print both:
        printCurrentPlan(*planned, "Planned");
        printCurrentPlan(*performed, "Performed");
    }

    return true;
}

void recomputeGraphWithActualValues(Graph& g, std::vector<std::tuple<int, int, float>> performed) {
    for (int i = 0; i < performed.size(); i++) {
        // Extract values from step
        int leftIdx = std::get<0>(performed[i]);
        int rightIdx = std::get<1>(performed[i]);
        float actualVal = std::get<2>(performed[i]);
    
        // Contract edge in graph
        int edgeName = g.generateEdgeIdx(g.vertices[leftIdx], g.vertices[rightIdx]);
        g.contractEdge(edgeName, actualVal);
    }
}

std::vector<std::tuple<int, int, float>> NextWindowNNGreedy(Graph g, std::vector<std::tuple<int, int, float>> lockedSteps, int window = 1) {
    // Recompute locked window in the graph
    for (int i = 0; i < lockedSteps.size(); i++) {
        int leftIdx = std::get<0>(lockedSteps[i]);
        int rightIdx = std::get<1>(lockedSteps[i]);
    
        // Contract edge in graph
        int edgeName = g.generateEdgeIdx(g.vertices[leftIdx], g.vertices[rightIdx]);
        g.contractEdge(edgeName);
    }

    std::vector<std::tuple<int, int, float>> nextSteps = {};
    for (int i = 0; i < window && !g.doneWithContraction(); i++) {
        nextSteps.push_back(GreedyPlanNextStep(g));
    }

    return nextSteps;
}






