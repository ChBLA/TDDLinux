
#include <torch/script.h>

#include "Cir_import.h"
#include "dd/Tensor.hpp"
#include "dd/Package.hpp"

#include <vector>
#include <array>
#include <map>
#include <cmath>

using namespace std;

typedef torch::Tensor (* vTensorCombinator)(torch::Tensor l, torch::Tensor r, float p, int s);

bool debug_pytorch = true;
std::map<std::string, int> gateIndices = {{"h"s, 0}, {"cx"s, 1}, {"cnot"s, 1}, {"rz"s, 2}, {"rx"s, 3}, {"u3"s, 4}, {"ry"s, 5}, 
                                                {"s"s, 6}, {"x"s, 7}, {"cz"s, 8}, {"cy"s, 9}, {"y"s, 10}, {"z"s, 11}, {"t"s, 12}};
static std::map<std::string, int> gateSizes = {{"cx"s, 6}, {"cz"s, 6}, {"rz"s, 4}, {"s"s, 4}, {"h"s, 3}, {"y"s, 4}, {"z"s, 4}, {"x"s, 4}, 
                                            {"cy"s, 6}, {"t"s, 4}, {"ry"s, 3}, {"rx"s, 4}, {"u3"s, 4}};
const int maxGateIndex = 13;

void printTensor(torch::Tensor t) {
    printf("\nInput tensor: ");
    auto t_a = t.accessor<float,1>();
    for (int j = 0; j < t_a.size(0); j++) {
        printf("%f, ", t_a[j]);
    }
    printf("\n");
}

torch::jit::script::Module load_jit_module(std::string path) {
    printf("Trying to load %s\n", path.c_str());
    torch::jit::script::Module res;
    try {
        res = torch::jit::load(path);
    } catch (const c10::Error& e) {
        printf("Failed to load module");
    }

    return res;
}

torch::Tensor extractFromTDD(dd::TDD tdd, bool use_pred, std::unique_ptr<dd::Package<>>& dd) {
    std::vector<int> gateCounts(maxGateIndex + 1);
    torch::Tensor res = torch::zeros(3 + maxGateIndex, torch::kFloat32);

    for (int i = 0; i < tdd.gates.size(); i++) {
        std::string gate_name = tdd.gates[i].name;
        int gate_index = gateIndices[gate_name];
        res[gate_index + 2] += 1;
    }

    if (use_pred)
        res[0] = tdd.pred_size;
    else
        res[0] = std::log2(dd->size(tdd.e));

    res[1] = (int) tdd.key_2_index.size();

    if (true) {
        printTensor(res);
    }

    return res;
}

torch::Tensor makeTDDTensorFromValues(std::vector<dd::GateDef> gates, float size, int indices) {
    std::vector<int> gateCounts(maxGateIndex + 1);
    torch::Tensor res = torch::zeros(3 + maxGateIndex, torch::kFloat32);

    for (int i = 0; i < gates.size(); i++) {
        std::string gate_name = gates[i].name;
        int gate_index = gateIndices[gate_name];
        res[gate_index + 2] += 1;
    }

    res[0] = size;
    res[1] = indices;

    if (debug_pytorch)
        printTensor(res);

    return res;
}

torch::Tensor makeTDDTensorFromValues(std::string gate, int indices) {
    std::vector<int> gateCounts(maxGateIndex + 1);
    torch::Tensor res = torch::zeros(3 + maxGateIndex, torch::kFloat32);

    res[(gateIndices[gate]) + 2] += 1;
    res[0] = gateSizes[gate];
    res[1] = indices;

    if (debug_pytorch)
        printTensor(res);

    return res;
}



float applyModel(torch::jit::script::Module model, dd::TDD leftTDD, dd::TDD rightTDD, bool use_pred, std::unique_ptr<dd::Package<>>& dd) {
    // Extract required data from TDDs
    torch::Tensor leftData = extractFromTDD(leftTDD, use_pred, dd);
    torch::Tensor rightData = extractFromTDD(rightTDD, use_pred, dd);

    // Shared indices
    int sharedIndices = 0;
    for (int i = 0; i < leftTDD.key_2_index.size(); i++) {
        for (int j = 0; j < rightTDD.key_2_index.size(); j++) {
            if (!leftTDD.key_2_index[i].compare(rightTDD.key_2_index[j]))
                sharedIndices++;
        }
    }
    torch::Tensor sharedIndexTensor = torch::full({1}, (float)sharedIndices);
    printf("Shared indices: %f\n", (float)sharedIndices);
    // Make Pytorch compat. format
    //torch::Tensor inputTensor = torch::cat({leftData, rightData, sharedIndexTensor}, 0);
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(leftData);
    inputs.push_back(rightData);
    inputs.push_back(sharedIndexTensor);

    // Apply model
    at::Tensor output = model.forward(inputs).toTensor();

    return output.item<float>();
}

float applyModel(torch::jit::script::Module model, torch::Tensor leftData, torch::Tensor rightData, int sharedIndices) {
    torch::Tensor sharedIndexTensor = torch::full({1}, (float)sharedIndices);

    // Make Pytorch compat. format
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(leftData);
    inputs.push_back(rightData);
    inputs.push_back(sharedIndexTensor);

    // Apply model
    at::Tensor output = model.forward(inputs).toTensor();

    return output.item<float>();
}


// Tensor combiners
torch::Tensor format16Combiner(torch::Tensor left, torch::Tensor right, float pred, int sharedIndices) {
    torch::Tensor res = left + right;
    res[0] = pred;
    res[1] -= sharedIndices * 2;
    return res;
}


