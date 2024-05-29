#include <torch/script.h>

#include "Cir_import.h"
#include "dd/Tensor.hpp"
#include "dd/Package.hpp"

#include <vector>
#include <array>
#include <map>
#include <cmath>
#include"time.h"
#include <math.h>
#include <chrono>
#include <ctime>
#include <sys/time.h>

using namespace std;

typedef torch::Tensor (* vTensorCombinator)(torch::Tensor l, torch::Tensor r, float p, int s);

bool debug_pytorch = false;
std::map<std::string, int> gateIndices = {{"h"s, 0}, {"cx"s, 1}, {"cnot"s, 1}, {"rz"s, 2}, {"rx"s, 3}, {"u3"s, 4}, {"ry"s, 5}, 
                                                {"s"s, 6}, {"x"s, 7}, {"cz"s, 8}, {"cy"s, 9}, {"y"s, 10}, {"z"s, 11}, {"t"s, 12},
                                                {"cx_c"s, 1}, {"cx_t"s, 1}, {"cz_c"s, 8}, {"cy_c"s, 9}, {"cz_t"s, 8}, {"cy_t"s, 9}};
static std::map<std::string, int> gateSizes = {{"cx"s, 6}, {"cz"s, 6}, {"rz"s, 4}, {"s"s, 4}, {"h"s, 3}, {"y"s, 4}, {"z"s, 4}, {"x"s, 4}, 
                                            {"cy"s, 6}, {"t"s, 4}, {"ry"s, 3}, {"rx"s, 4}, {"u3"s, 4}, {"cx_c"s, 6}, {"cy_c"s, 6}, {"cz_c"s, 6},
                                            {"cx_t"s, 6}, {"cy_t"s, 6}, {"cz_t"s, 6}};
const int maxGateIndex = 13;
int stepCounter = 0;

torch::Device _device(torch::kCPU);

vector<std::string> splitString(const std::string& s, const std::string& seperator) {
	vector<std::string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()) {
		int flag = 0;
		while (i != s.size() && flag == 0) {
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[i] == seperator[x]) {
					++i;
					flag = 0;
					break;
				}
		}

		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0) {
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[j] == seperator[x]) {
					flag = 1;
					break;
				}
			if (flag == 0)
				++j;
		}
		if (i != j) {
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

float getMTime(struct timeval start, struct timeval end) {
    float mtime, seconds, useconds;  
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    //mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    mtime = (seconds) * 1000.0f + useconds/1000.0f;
    return mtime;
}


void printTensor(torch::Tensor t) {
    printf("\nInput tensor: ");
    auto t_a = t.accessor<float,1>();
    for (int j = 0; j < t_a.size(0); j++) {
        printf("%f, ", t_a[j]);
    }
    printf("\n");
}

void warmupModel(torch::jit::script::Module model) {
    for (int i = 0; i < 20; i++) {
        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(torch::rand({5 * (i+1), 16}).to(_device));
        inputs.push_back(torch::rand({5 * (i+1), 16}).to(_device));
        inputs.push_back(torch::rand({5 * (i+1), 1}).to(_device));

        at::Tensor output = model.forward(inputs).toTensor();
    }
}

torch::jit::script::Module load_jit_module(std::string path) {
    std::string actual_path = path + ".pt";
    printf("Trying to load %s\n", actual_path.c_str());
    torch::jit::script::Module res;
    try {
        res = torch::jit::load(actual_path, _device);
        //res.to(_device); // torch::Device device(torch::kCPU); res->to(device)
    } catch (const c10::Error& e) {
        printf("Failed to load module");
    }

    //printf("Model is on: %s", res.device());
    // printf("Parameters are on:\n");
    // for (const auto& param : res.named_parameters()) {
    //     std::cout << "Parameter " << param.name << " is on device: " << param.value.device() << std::endl;
    // }
    warmupModel(res);
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

    std::string gateNoParam = splitString(gate, "(")[0];

    if (gateIndices.find(gateNoParam) == gateIndices.end() || gateSizes.find(gateNoParam) == gateSizes.end()) {
        res[0] = 2;
        res[1] = indices;
    }

    res[(gateIndices[gateNoParam]) + 2] = 1;
    res[0] = std::log2(gateSizes[gateNoParam]);
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
    // Make Pytorch compat. format
    //torch::Tensor inputTensor = torch::cat({leftData, rightData, sharedIndexTensor}, 0);
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(leftData.to(_device));
    inputs.push_back(rightData.to(_device));
    inputs.push_back(sharedIndexTensor.to(_device));
    //inputs.to(torch::kCUDA);
    // Apply model
    //printf("Model (in apply) is on: %s", model.device());
    //printf("Inputs is on: %s", inputs.device());
    at::Tensor output = model.forward(inputs).toTensor().to(torch::kCPU);

    return output.item<float>();
}

// float applyModel(torch::jit::script::Module model, torch::Tensor leftData, torch::Tensor rightData, int sharedIndices) {
//     torch::Tensor sharedIndexTensor = torch::full({1}, (float)sharedIndices);

//     // Make Pytorch compat. format
//     std::vector<torch::jit::IValue> inputs;
//     inputs.push_back(leftData.to(_device));
//     inputs.push_back(rightData.to(_device));
//     inputs.push_back(sharedIndexTensor.to(_device));

//     // Apply model
//     at::Tensor output = model.forward(inputs).toTensor().to(torch::kCPU);

//     return output.item<float>();
// }

std::vector<float> applyModel(torch::jit::script::Module model, std::vector<torch::Tensor> leftDatas, 
                                std::vector<torch::Tensor> rightDatas, std::vector<int> sharedIndices) {
    std::vector<torch::Tensor> sharedIndexTensor = {};
    for (int i = 0; i < sharedIndices.size(); i++) {
        auto intermediateTensor = torch::full({1}, (float)sharedIndices[i]);
        sharedIndexTensor.push_back(intermediateTensor);
    }
    // auto opts = torch::TensorOptions().dtype(torch::kFloat32);
    // auto sharedIndexTensor = torch::from_blob(floatedSIs.data(), {1, floatedSIs.size()}, opts);
    //sharedIndexTensor.to(_device);
    //struct timeval start, end;



    //gettimeofday(&start, NULL);
    std::vector<torch::jit::IValue> inputs;
    auto leftStacked = torch::stack(leftDatas, 0);
    auto rightStacked = torch::stack(rightDatas, 0);
    auto sharedStacked = torch::stack(sharedIndexTensor, 0);
    //gettimeofday(&end, NULL);
	
    //printf("Step %d: stacking time = %f\n", stepCounter, getMTime(start, end));

    // leftStacked.to(_device);
    // rightStacked.to(_device);
    // sharedStacked.to(_device);
    //gettimeofday(&start, NULL);
    inputs.push_back(leftStacked.to(_device));
    inputs.push_back(rightStacked.to(_device));
    inputs.push_back(sharedStacked.to(_device));
    //inputs.push_back(sharedIndexTensor.unsqueeze(1));
    //gettimeofday(&end, NULL);
	
    //printf("Step %d: device moving time = %f\n", stepCounter, getMTime(start, end));
    // printf("Device left: %s\n", leftStacked.device());
    // printf("Device right: %s\n", rightStacked.device());
    // printf("Device shared: %s\n", sharedStacked.device());

    //printf("Step %d: %d batched\n", stepCounter, sharedIndices.size());
    //gettimeofday(&start, NULL);

    at::Tensor output = model.forward(inputs).toTensor();
    //gettimeofday(&end, NULL);
    //printf("Step %d: model time = %f\n", stepCounter, getMTime(start, end));

    //gettimeofday(&start, NULL);
    output = output.to(torch::kCPU);
    //gettimeofday(&end, NULL);
    //printf("Step %d: output to cpu time = %f\n\n", stepCounter++, getMTime(start, end));

    std::vector<float> v(output.data_ptr<float>(), output.data_ptr<float>() + output.numel());

    return v;
    
    // for (int i = 0; i < leftDatas.size(); i++) {
    //     if (v[i] < 0.1f || !&v[i] || std::isnan(v[i])) {
    //         printf("Batch %d:\n", i);
    //         std::cout << leftDatas[i];
    //         printf("\n");
    //         std::cout << rightDatas[i];
    //         printf("\n");
    //         printf("Shared indices: %d\n", sharedIndices[i]);
    //         printf("Output: %f\n", v[i]);

    //         printf("\n");
    //     }
    // }

    // return v;
}


// Tensor combiners
torch::Tensor format16Combiner(torch::Tensor left, torch::Tensor right, float pred, int sharedIndices) {
    torch::Tensor res = left + right;
    res[0] = pred;
    res[1] -= sharedIndices * 2;
    return res.to(_device);
}


