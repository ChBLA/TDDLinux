#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <queue>
#include <algorithm>
#include <stdio.h>
#include"time.h"
#include <math.h>
#include <chrono>
#include <ctime>
#include <sys/time.h>

using namespace std;

const bool debug_qplanning = false;

struct QEdge {
    int id;
    int vertices[2];
    int sharedIndices;
    float time;

    friend bool operator<(const QEdge& l, const QEdge& r) {
        if (l.id < r.id) return true;
        return false;
    }

    friend bool operator>(const QEdge& l, const QEdge& r) {
        return r < l;
    }

    friend bool operator<=(const QEdge& l, const QEdge& r) {
        return !(l > r);
    }

    friend bool operator>=(const QEdge& l, const QEdge& r) {
        return !(l < r);
    }

    friend bool operator==(const QEdge& l, const QEdge& r) {
        return l.vertices[0] == r.vertices[0] 
                && l.vertices[1] == r.vertices[1] 
                && l.sharedIndices == r.sharedIndices;
    }

    friend bool operator!=(const QEdge& l, const QEdge& r) {
        return !(l == r);
    }

    bool containsVertex(int vertexId) {
        return vertices[0] == vertexId || vertices[1] == vertexId;
    }

    QEdge copy() {
        QEdge copy;
        copy.id = id;
        copy.vertices[0] = vertices[0];
        copy.vertices[1] = vertices[1];
        copy.sharedIndices = sharedIndices;
        copy.time = time;
        return copy;
    }

};

std::string edgeToString(QEdge& e) {
    return "id: " + std::to_string(e.id) + ", vertices: [" + std::to_string(e.vertices[0]) + ", " + std::to_string(e.vertices[1]) + "], sharedIndices: " + std::to_string(e.sharedIndices);
}



struct QVertex {
    int idx;
    std::vector<int> edges;

    QVertex copy() {
        QVertex copy;
        copy.edges = edges;
        copy.idx = idx;
        return copy;
    }

};

std::string vertexToString(QVertex& v) {
    string edges = "[";
    for (int i = 0; i < v.edges.size(); i++)
        edges += std::to_string(v.edges[i]) + ", ";
    
    return "id: " + std::to_string(v.idx) + ", edges: " + edges + "]";
}

class QGraph {
    // Private members


public:
    std::map<int, QVertex> vertices;
    std::map<int, QEdge> edges;
    int maxVertexId;
    int currentStep;

    float timeForCurrentStep;
    std::vector<float> updateTime;
    std::vector<float> contractionTime;



    QGraph(std::map<int, QVertex> vertices, std::map<int, QEdge> edges, const int maxVertexId)
            : vertices(vertices), edges(edges), maxVertexId(maxVertexId)
        {
            timeForCurrentStep = 0;
            currentStep = 0;
            updateTime = {};
            contractionTime = {};
        }    

    int generateEdgeIdx(QVertex v1, QVertex v2) {
        return (v1.idx < v2.idx ? v1.idx * this->maxVertexId + v2.idx : v2.idx * this->maxVertexId + v1.idx) + this->currentStep * this->maxVertexId * this->maxVertexId;
    }

    std::vector<int> contractEdge(int edgeIdx) {
        this->currentStep++;
        QEdge edge = this->edges[edgeIdx];
        QVertex left = this->vertices[edge.vertices[0]];
        QVertex right = this->vertices[edge.vertices[1]];
        if (debug_planning) {
            printf("Contracting edge: %d with vertices: %d %d\n", edgeIdx, edge.vertices[0], edge.vertices[1]);
        }

        // Prepare new vertex
        this->contractionTime.push_back(edge.time);
        this->updateTime.push_back(this->timeForCurrentStep);
        this->timeForCurrentStep = 0;

        // Delete edge from result
        right.edges.erase(std::remove(right.edges.begin(), right.edges.end(), edgeIdx), right.edges.end());
        
        if (debug_planning)
            printf("Right vertex just after deleting: %s\n", vertexToString(right).c_str());

        std::vector<int> newEdges = {};

        for (int i = 0; i < right.edges.size(); i++) {
            int old_edge = right.edges[i];
            int nonRightVertexIdx = this->edges[old_edge].vertices[0] != right.idx ? this->edges[old_edge].vertices[0] : this->edges[old_edge].vertices[1];
            int new_edge_name = this->generateEdgeIdx(right, this->vertices[nonRightVertexIdx]);
            
            
            right.edges[i] = new_edge_name;
            newEdges.push_back(new_edge_name);

            this->edges[new_edge_name] = this->edges[old_edge];
            this->edges[new_edge_name].id = new_edge_name;

            for (int j = 0; j < this->vertices[nonRightVertexIdx].edges.size(); j++) {
                if (this->vertices[nonRightVertexIdx].edges[j] == old_edge)
                    this->vertices[nonRightVertexIdx].edges[j] = new_edge_name;
            }

            this->edges.erase(old_edge);
        }



        for (int i = 0; i < left.edges.size(); i++) {
            int nonLeftVertexIdx = this->edges[left.edges[i]].vertices[0] != left.idx ? this->edges[left.edges[i]].vertices[0] : this->edges[left.edges[i]].vertices[1];
            int new_edge_name = this->generateEdgeIdx(right, this->vertices[nonLeftVertexIdx]);
            
            if (debug_planning)
                printf("New edge name: %d, with maxVertexId=%d and nonLeftVertixId=%d and actual idx for both: %d %d\n", new_edge_name, this->maxVertexId, nonLeftVertexIdx, right.idx, this->vertices[nonLeftVertexIdx].idx);
            
            if (left.edges[i] != edge.id && this->edges.find(new_edge_name) == this->edges.end()) {
                if (debug_planning)
                    printf("New edge does not exist already\n");
                right.edges.push_back(new_edge_name);
                newEdges.push_back(new_edge_name);
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

        // Remove contracted edge
        const auto it = this->edges.find(edgeIdx);
        if (it != this->edges.end())
            this->edges.erase(it);

        this->vertices[right.idx] = right;
        this->vertices[left.idx] = {left.idx, {}};

        return newEdges;
    }

    

    bool doneWithContraction() {
        return this->edges.size() < 1;
    }

    int edgeCount() {
        return this->edges.size();
    }

    QGraph copy() {
        std::map<int, QVertex> copyVertices = {};
        for (auto it = this->vertices.begin(); it != this->vertices.end(); ++it) {
            int key = it->first;
            QVertex& val = it->second;
            copyVertices[key] = val.copy();
        }

        std::map<int, QEdge> copyEdges = {};
        for (auto it = this->edges.begin(); it != this->edges.end(); ++it) {
            int key = it->first;
            QEdge& val = it->second;
            copyEdges[key] = val.copy();
        }

        return QGraph(copyVertices, copyEdges, maxVertexId);
    }
};




QGraph initialiseQGraph(std::map<int, gate> gate_set, std::map<int, std::vector<dd::Index>> index_set, std::vector<std::tuple<int, int>> pythonEdges) {
    std::map<int, QEdge> edges = {};
    std::map<int, QVertex> vertices = {};

    int maxVertexId = 1;
    for (int i = 0; i < pythonEdges.size(); i++) {
        if (std::get<0>(pythonEdges[i]) > maxVertexId)
            maxVertexId = std::get<0>(pythonEdges[i]);        
        if (std::get<1>(pythonEdges[i]) > maxVertexId)
            maxVertexId = std::get<1>(pythonEdges[i]);
    }
    maxVertexId++;

    for (int i = 0; i < pythonEdges.size(); i++) {
        QEdge new_edge;
        new_edge.id = std::get<0>(pythonEdges[i]) * maxVertexId + std::get<1>(pythonEdges[i]);
        if (edges.find(new_edge.id) != edges.end()) {
            edges[new_edge.id].sharedIndices++;
            continue;
        }
        new_edge.vertices[0] = std::get<0>(pythonEdges[i]);
        new_edge.vertices[1] = std::get<1>(pythonEdges[i]);
        
        int leftIdx = plan_offset[std::get<0>(pythonEdges[i])];
        int rightIdx = plan_offset[std::get<1>(pythonEdges[i])];

        int sharedIndices = 0;
        for (int j = 0; j < index_set[leftIdx].size(); j++) {
            for (int k = 0; k < index_set[rightIdx].size(); k++) {
                if (index_set[leftIdx][j] == index_set[rightIdx][k])
                    sharedIndices++;
            }
        }
        new_edge.sharedIndices = sharedIndices;

        edges[new_edge.id] = new_edge;

        int idx = std::get<0>(pythonEdges[i]);
        if (vertices.find(idx) != vertices.end()) {
            vertices[idx].edges.push_back(new_edge.id);
        } else {
            QVertex vertex;
            int vertIdx = plan_offset[idx];
            vertex.idx = idx;
            vertex.edges = { new_edge.id };
            vertices[idx] = vertex;
        }

        idx = std::get<1>(pythonEdges[i]);
        if (vertices.find(idx) != vertices.end()) {
            vertices[idx].edges.push_back(new_edge.id);
        } else {
            QVertex vertex;
            int vertIdx = plan_offset[idx];
            vertex.idx = idx;
            vertex.edges = { new_edge.id };
            vertices[idx] = vertex;
        }

    }


    return QGraph(vertices, edges, maxVertexId);
}

std::vector<std::tuple<int, int>> QueuePlan(QGraph& g) {
    std::vector<std::tuple<int, int>> plan = {};
    std::queue<int> edgesTodo = {};
    //printf("Initially adding: ");
    for (auto it = g.edges.begin(); it != g.edges.end(); ++it) {
        //printf("%d, ", it->first);
        edgesTodo.push(it->first);
    }
    //printf("\n");

    //printf("Starting the contraction of the graph\n");
    while (!g.doneWithContraction() && !edgesTodo.empty()) {
        auto nextStep = edgesTodo.front();
        edgesTodo.pop();
        //printf("Next step: %d\n", nextStep);
        if (g.edges.find(nextStep) != g.edges.end()) {
            //printf("\t is valid edge\n");

            auto leftVertex = g.edges[nextStep].vertices[0];
            auto rightVertex = g.edges[nextStep].vertices[1];

            std::vector<int> newEdges = g.contractEdge(nextStep);
            
            //printf("Adding to queue: ");
            for (int i = 0; i < newEdges.size(); i++) {
                //printf("%d, ", newEdges[i]);
                edgesTodo.push(newEdges[i]);
            }

            //printf("Adding (%d, %d) to plan\n", leftVertex, rightVertex);
            plan.push_back({leftVertex, rightVertex});
        }
    }

    return plan;
}


void printCurrentQPlan(std::vector<std::tuple<int,int>> plan, std::string name) {
    std::string res = name + ": ";
    for (int i = 0; i < plan.size(); i++) {
        res += "\t(" + std::to_string(std::get<0>(plan[i])) + ", " + std::to_string(std::get<1>(plan[i])) + ")   ";
    }
    res += "\n";

    printf(res.c_str());
}






