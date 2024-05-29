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

const bool l_debug_planning = false;
int counter = 0;
std::string res_name = "test_of_dj";

struct Lgate {
	std::string name;
	short int qubits[2];
	//std::vector<dd::Index> index_set;
};
std::map<int, int> l_plan_offset;

struct LEdge {
    int id;
    int vertices[2];
    float weight;
    int sharedIndices;
    float time;
    dd::TDD resultingTDD;

    friend bool operator<(const LEdge& l, const LEdge& r) {
        if (l.weight < r.weight) return true;
        return false;
    }

    friend bool operator>(const LEdge& l, const LEdge& r) {
        return r < l;
    }

    friend bool operator<=(const LEdge& l, const LEdge& r) {
        return !(l > r);
    }

    friend bool operator>=(const LEdge& l, const LEdge& r) {
        return !(l < r);
    }

    friend bool operator==(const LEdge& l, const LEdge& r) {
        return l.vertices[0] == r.vertices[0] 
                && l.vertices[1] == r.vertices[1] 
                && l.weight == r.weight 
                && l.sharedIndices == r.sharedIndices;
    }

    friend bool operator!=(const LEdge& l, const LEdge& r) {
        return !(l == r);
    }

    LEdge copy() {
        LEdge copy;
        copy.id = id;
        copy.vertices[0] = vertices[0];
        copy.vertices[1] = vertices[1];
        copy.weight = weight;
        copy.sharedIndices = sharedIndices;
        copy.resultingTDD = resultingTDD.copy();
        copy.time = time;
        return copy;
    }

};

std::string lEdgeToString(LEdge& e) {
    return "id: ";// + std::to_string(e.id);// + ", weight: " + std::to_string(e.weight) + ", vertices: [" + std::to_string(e.vertices[0]) + ", " + std::to_string(e.vertices[1]) + "], sharedIndices: " + std::to_string(e.sharedIndices);
}


struct LVertex {
    int idx;
    std::vector<int> edges;
    dd::TDD tdd;

    LVertex copy() {
        LVertex copy;
        copy.edges = edges;
        copy.idx = idx;
        copy.tdd = tdd.copy();
        return copy;
    }

};

std::string lVertexToString(LVertex& v) {
    string edges = "[";
    for (int i = 0; i < v.edges.size(); i++)
        edges += std::to_string(v.edges[i]) + ", ";
    
    return "id: " + std::to_string(v.idx) + ", edges: " + edges + "]";
}

class LGraph {
    // Private members


public:
    std::map<int, LVertex> vertices;
    std::map<int, LEdge> edges;
    int maxVertexId;
    float timeForCurrentStep;
    std::vector<float> updateTime;
    std::vector<float> contractionTime;
    std::unique_ptr<dd::Package<>>& dd;

    std::queue<int> workingQueue;

    LGraph(std::map<int, LVertex> vertices, std::map<int, LEdge> edges, std::queue<int> workingQueue, const int maxVertexId, std::unique_ptr<dd::Package<>>& dd)
            : vertices(vertices), edges(edges), workingQueue(workingQueue), maxVertexId(maxVertexId), dd(dd)
        {
            timeForCurrentStep = 0;
            updateTime = {};
            contractionTime = {};
            if (!this->workingQueue.empty())
                this->updateEdges();

        }    

    int generateEdgeIdx(int v1idx, int v2idx) {
        return v1idx < v2idx ? v1idx * this->maxVertexId + v2idx : v2idx * this->maxVertexId + v1idx;
    }

    int generateEdgeIdx(LVertex v1, LVertex v2) {
        return v1.idx < v2.idx ? v1.idx * this->maxVertexId + v2.idx : v2.idx * this->maxVertexId + v1.idx;
    }

    void contractEdge(int edgeIdx, float actualVal=0.0f) {
        LEdge edge = this->edges[edgeIdx];
        LVertex left = this->vertices[edge.vertices[0]];
        LVertex right = this->vertices[edge.vertices[1]];
        if (l_debug_planning) {
            printf("Contracting edge: %d with vertices: %d %d\n", edgeIdx, edge.vertices[0], edge.vertices[1]);
            
            std::string folder_name = std::string("cpp_debugging/contraction_") + std::to_string(counter) + "/";
            std::filesystem::create_directory(folder_name);
			dd::export2Dot(left.tdd.e, folder_name + res_name + "_v" + std::to_string(edge.vertices[0]));
			dd::export2Dot(right.tdd.e, folder_name + res_name + "_h" + std::to_string(edge.vertices[1]));
        }
        // Prepare new vertex
        right.tdd = edge.resultingTDD;
        this->contractionTime.push_back(edge.time);
        this->updateTime.push_back(this->timeForCurrentStep);
        this->timeForCurrentStep = 0;

        if (l_debug_planning) {
            std::string folder_name = std::string("cpp_debugging/contraction_") + std::to_string(counter++) + "/";
            dd::export2Dot(right.tdd.e, folder_name + res_name + "_r" + std::to_string(edgeIdx));
        }

        // Delete edge from result
        right.edges.erase(std::remove(right.edges.begin(), right.edges.end(), edgeIdx), right.edges.end());
        
        if (l_debug_planning)
            printf("Right vertex just after deleting: %s\n", lVertexToString(right).c_str());

        for (int i = 0; i < left.edges.size(); i++) {
            int nonLeftVertexIdx = this->edges[left.edges[i]].vertices[0] != left.idx ? this->edges[left.edges[i]].vertices[0] : this->edges[left.edges[i]].vertices[1];
            int new_edge_name = this->generateEdgeIdx(right, this->vertices[nonLeftVertexIdx]);
            
            if (l_debug_planning)
                printf("New edge name: %d, with maxVertexId=%d and nonLeftVertixId=%d and actual idx for both: %d %d\n", new_edge_name, this->maxVertexId, nonLeftVertexIdx, right.idx, this->vertices[nonLeftVertexIdx].idx);
            
            if (left.edges[i] != edge.id && this->edges.find(new_edge_name) == this->edges.end()) {
                if (l_debug_planning)
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
                if (l_debug_planning)
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
            if (l_debug_planning && false) {
                printf("Updating edge: %d\n", elemIdx);
                printf("Total edges: %d\n", this->edges.size());
                for (auto& x : this->edges) {
                    printf("Edge %d: %d\n", x.first, x.second.id);
                    printf("Edge %d: %d\n", x.first, x.second.vertices[0]);
                    printf("Edge %d: %d\n", x.first, x.second.vertices[1]);
                    printf("Edge %d: %d\n", x.first, x.second.sharedIndices);
                    printf("Edge %d: %d\n", x.first, x.second.weight);
                    //printf("Edge %d: %d\n", x.first, lEdgeToString(x.second));
                }
            }
            LEdge edge = this->edges[elemIdx];
            struct timeval start, end;
	        gettimeofday(&start, NULL);
            
            edge.resultingTDD = realContract(this->vertices[edge.vertices[0]], this->vertices[edge.vertices[1]]);
            
            gettimeofday(&end, NULL);
            edge.time = getMTime(start, end);
            
            edge.weight = this->dd->size(edge.resultingTDD.e);
            this->edges[elemIdx] = edge;
            this->workingQueue.pop();
            this->timeForCurrentStep += edge.time;
        }
    }

    dd::TDD realContract(LVertex left, LVertex right) {
        return this->dd->cont(left.tdd, right.tdd);
    }

    bool doneWithContraction() {
        return this->edges.size() < 1;
    }

    int edgeCount() {
        return this->edges.size();
    }

    LGraph copy() {
        std::map<int, LVertex> copyVertices = {};
        for (auto it = this->vertices.begin(); it != this->vertices.end(); ++it) {
            int key = it->first;
            LVertex& val = it->second;
            copyVertices[key] = val.copy();
        }

        std::map<int, LEdge> copyEdges = {};
        for (auto it = this->edges.begin(); it != this->edges.end(); ++it) {
            int key = it->first;
            LEdge& val = it->second;
            copyEdges[key] = val.copy();
        }

        return LGraph(copyVertices, copyEdges, workingQueue, maxVertexId, this->dd);
    }
};




LGraph initialiseLGraph(std::vector<dd::TDD> gateTDDs, std::map<int, std::vector<dd::Index>> index_set, std::vector<std::tuple<int, int>> pythonEdges, std::unique_ptr<dd::Package<>>& dd) {
    std::map<int, LEdge> edges = {};
    std::map<int, LVertex> vertices = {};
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
        LEdge new_edge;
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
        new_edge.resultingTDD = {};
        edges[new_edge.id] = new_edge;
        workingQueue.push(new_edge.id);

        int idx = std::get<0>(pythonEdges[i]);
        if (vertices.find(idx) != vertices.end()) {
            vertices[idx].edges.push_back(new_edge.id);
        } else {
            LVertex vertex;
            int vertIdx = plan_offset[idx];
            vertex.idx = idx;
            vertex.tdd = gateTDDs[vertIdx];
            vertex.edges = { new_edge.id };
            vertices[idx] = vertex;
        }

        idx = std::get<1>(pythonEdges[i]);
        if (vertices.find(idx) != vertices.end()) {
            vertices[idx].edges.push_back(new_edge.id);
        } else {
            LVertex vertex;
            int vertIdx = plan_offset[idx];
            vertex.idx = idx;
            vertex.tdd = gateTDDs[vertIdx];
            vertex.edges = { new_edge.id };
            vertices[idx] = vertex;
        }

    }

    return LGraph(vertices, edges, workingQueue, maxVertexId, dd);
}

std::tuple<int, int, float, float> LookAheadNextStep(LGraph& g) {
    int min_edge = -1;
    for (auto it = g.edges.begin(); it != g.edges.end(); ++it) {
        int key = it->first;
        LEdge& val = it->second;
        if (min_edge == -1 || g.edges[min_edge] > val) {
            min_edge = key;
        }
    }

    float weight = g.edges[min_edge].weight;
    std::tuple<int, int> step = {g.edges[min_edge].vertices[0], g.edges[min_edge].vertices[1]};
    float edge_time = g.edges[min_edge].time;
    g.contractEdge(min_edge);

    return {std::get<0>(step), std::get<1>(step), weight, edge_time};
}

std::vector<std::tuple<int, int, float, float>> LookAheadPlan(std::vector<dd::TDD> gateTDDs, std::map<int, std::vector<dd::Index>> index_set, 
                                                std::vector<std::tuple<int, int>> pythonEdges, std::unique_ptr<dd::Package<>>& dd) {
    LGraph g = initialiseLGraph(gateTDDs, index_set, pythonEdges, dd);
    std::vector<std::tuple<int, int, float, float>> plan = {};
    int edgeCount = g.edges.size();

    for (int i = 0; i < edgeCount - 1; i++) {
        plan.push_back(LookAheadNextStep(g));
    }

    return plan;
}
