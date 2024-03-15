// TDDLinux.cpp : Defines the entry point for the application.
//

#include "TDDLinux.h"
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>


using namespace std;

int save_data();


int main3(int argc, char* argv[])
{
	xt::xarray<double> arr1
	{ {1.0, 2.0, 3.0},
	  { 2.0, 5.0, 7.0 },
	  { 2.0, 5.0, 7.0 } };

	xt::xarray<dd::ComplexValue> arr2
	{ {1, 2}, { 3,4 }, {5,6} };

	xt::xarray<double> arr3
	{ { {1.0, 2.0, 3.0},
		{ 2.0, 5.0, 7.0 },
		{ 2.0, 5.0, 7.0 } },
		{ {1.0, 2.0, 3.0},
		{ 2.0, 5.0, 7.0 },
		{ 2.0, 5.0, 7.0 } } };

	std::cout << xt::view(arr3, xt::all(), 0, 1) << std::endl;

	auto shape = arr2.shape();


	std::cout << arr2.size() << std::endl;

	auto dd = std::make_unique<dd::Package<>>(100);

	/*xt::xarray<int> U = { {{{1, 0}, {0, 1}}, {{0, 0}, {0, 0}}}, {{{0, 0}, {0, 0}}, {{0, 1}, {1, 0}}} };
	dd::ComplexValue v = { 1,0 };*/

	//xt::xarray<dd::ComplexValue> K = { {{{1, 0}, {0, 1}}, {{0, 0}, {0, 0}}}, {{{0, 0}, {0, 0}}, {{0, 1}, {1, 0}}} };
	//xt::xarray<dd::ComplexValue> K = { {{1, 0}, {0, 1}}, {{0, 0}, {0, 0}} };
	//xt::xarray<dd::ComplexValue> K = { {1, 0}, {0, 1} };
	xt::xarray<dd::ComplexValue> K1 = { { {1, 0}, {1, 0} }, { {1, 0}, {-1, 0} } }; // works
	xt::xarray<dd::ComplexValue> K2 = { { {1, 0}, {1, 0} }, { {1, 0}, {-1, 0} } }; // works
	//xt::xarray<dd::ComplexValue> K = { {{1, 0}, {0, 1}}, {{1, 0}, {0, 0}} };
	//xt::xarray<dd::ComplexValue> K = (xt::xarray<dd::ComplexValue>) U;
	//dd::Tensor ts = { K,{{"a", 1},{"b", 2},{"c", 3}},"abc" };
	//dd::Tensor ts = { K,{{"a", 1},{"b", 2}},"abc" };
	//dd::Tensor ts = { K,{{"a", 1},{"b", 2}},"abc" };
	//dd::Tensor ts = { K,{{"a", 1},{"b", 2}},"abc" }; // works
	dd::Tensor ts1 = { K1,{{"a", 1}, {"c", 2}},"a" };
	dd::Tensor ts2 = { K2,{{"c", 2}, {"b", 3}},"b" };
	dd->varOrder = { {"a", 1}, {"c", 2}, {"b", 3} };
	auto tdd1 = dd->Tensor_2_TDD(ts1);
	auto tdd2 = dd->Tensor_2_TDD(ts2);

	auto res = dd->cont(tdd1, tdd2);

	bool isIden = dd->isTDDIdentity(res, false, 1);

	dd::export2Dot(tdd1.e, "tdd1");
	dd::export2Dot(tdd2.e, "tdd2");
	dd::export2Dot(res.e, "tddRes");

	return 0;
}

int main124() {
	string a = "a";
	std::cout << a << std::endl;
	return 0;
}

int main() {
	string path2 = "Benchmarks/";
	string file_name = "test.qasm";
	int n = get_qubits_num(path2 + file_name);
	auto dd = std::make_unique<dd::Package<>>(3 * n);
	std::cout << "File name:" << file_name << std::endl;
	std::vector<std::tuple<int, int>> plan = getDefaultPlan(get_gates_num(path2 + file_name));
	dd::TDD res = plannedContractionOnCircuitFromFile(path2, plan, file_name, dd);
	std::cout << "Done" << std::endl;

}

// int main() {
// 	string circuit = "OPENQASM 2.0;\n include "qelib1.inc";\n qreg q[10];\n z q[6];\n h q[2];\n h q[7];\n z q[5];\n y q[6];\n y q[0];\n z q[0];\n h q[4];\n y q[1];\n x q[8];\n z q[6];\n s q[3];\n z q[7];\n s q[1];\n z q[6];\n s q[9];\n x q[1];\n z q[2];\n x q[9];\n h q[4];\n cx q[6],q[1];\n z q[7];\n y q[5];\n s q[9];\n z q[9];\n y q[9];\n h q[4];\n x q[9];\n x q[5];\n z q[8];\n x q[9];\n z q[4];\n s q[8];\n y q[7];\n cx q[9],q[3];\n cx q[9],q[1];\n s q[0];\n cx q[0],q[5];\n h q[2];\n h q[3];\n x q[5];\n z q[9];\n z q[1];\n s q[0];\n x q[4];\n s q[6];\n cx q[3],q[8];\n y q[5];\n s q[8];\n h q[6];\n z q[0];\n y q[9];\n x q[6];\n h q[7];\n x q[6];\n z q[7];\n s q[5];\n cx q[7],q[9];\n h q[7];\n x q[9];\n z q[2];\n cx q[6],q[4];\n s q[6];\n y q[1];\n z q[3];\n y q[5];\n x q[2];\n h q[2];\n s q[5];\n z q[0];\n x q[6];\n cx q[4],q[0];\n s q[5];\n cx q[2],q[4];\n z q[1];\n y q[0];\n x q[3];\n y q[2];\n z q[5];\n s q[6];\n s q[5];\n x q[4];\n y q[5];\n s q[4];\n s q[7];\n x q[4];\n y q[4];\n y q[3];\n z q[2];\n h q[1];\n s q[2];\n h q[6];\n s q[1];\n y q[1];\n cx q[4],q[9];\n cx q[9],q[6];\n cx q[4],q[1];\n x q[7];\n x q[5];\n z q[5];\n x q[7];\n h q[0];\n s q[4];\n x q[5];\n cx q[7],q[9];\n z q[7];\n cx q[0],q[6];\n y q[8];\n y q[8];\n cx q[8],q[1];\n y q[4];\n cx q[6],q[8];\n cx q[7],q[6];\n cx q[3],q[7];\n y q[6];\n h q[2];\n s q[5];\n x q[7];\n z q[0];\n y q[8];\n cx q[6],q[3];\n cx q[2],q[3];\n x q[9];\n h q[7];\n h q[4];\n z q[7];\n h q[5];\n x q[7];\n h q[8];\n y q[9];\n s q[7];\n x q[7];\n s q[9];\n s q[8];\n z q[9];\n x q[8];\n h q[7];\n h q[3];\n z q[8];\n h q[4];\n x q[3];\n h q[8];\n h q[5];\n x q[2];\n z q[6];\n cx q[5],q[0];\n h q[9];\n y q[3];\n z q[0];\n z q[9];\n cx q[0],q[8];\n s q[0];\n h q[4];\n cx q[4],q[5];\n x q[2];\n z q[8];\n z q[5];\n cx q[1],q[9];\n s q[8];\n cx q[7],q[1];\n z q[4];\n y q[7];\n s q[8];\n cx q[5],q[4];\n s q[8];\n z q[5];\n cx q[3],q[2];\n z q[9];\n x q[1];\n s q[1];\n x q[5];\n h q[0];\n s q[7];\n x q[8];\n cx q[6],q[0];\n y q[2];\n z q[7];\n x q[9];\n z q[0];\n y q[4];\n s q[7];\n z q[3];\n s q[8];\n x q[6];\n s q[1];\n s q[4];\n h q[2];\n h q[9];\n h q[9];\n s q[7];\n h q[0];\n cx q[8],q[2];\n cx q[1],q[8];\n h q[1];\n s q[7];\n y q[9];\n y q[7];\n z q[1];\n h q[8];\n h q[2];\n ";
// 	string plan = 

// }

int main2() {

	//qc::QuantumComputation qc1{};

	//string path = "Benchmarks/test.qasm";
	//qc1.import(path, qc::Format::OpenQASM);
	//const qc::MatrixDD dd1 = buildFunctionality(&qc1, dd);
	//dd->printInformation();
	//serialize(dd1, "output.ser");


	string path2 = "Benchmarks/";

	string file_name = "test.qasm";
	int* nodes;
	int n = get_qubits_num(path2 + file_name);
	auto dd = std::make_unique<dd::Package<>>(3 * n);
	clock_t   start_t, finish_t;
	double time_t;
	std::cout << "File name:" << file_name << std::endl;
	start_t = clock();
	nodes = Simulate_with_tdd(path2, file_name, dd);
	finish_t = clock();
	time_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
	std::cout << "Time:" << time_t << std::endl;
	std::cout << "Nodes max:" << *nodes << std::endl;
	std::cout << "Nodes Final:" << *(nodes + 1) << std::endl;
	std::cout << "===================================" << std::endl;


	//std::cout << "File name:" << file_name << std::endl;
	//auto dd2 = std::make_unique<dd::Package<>>(3 * n);
	//start_t = clock();
	//nodes = Simulate_with_partition1(path2, file_name,dd2);
	//finish_t = clock();
	//time_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;

	//std::cout << "Time:" << time_t << std::endl;
	//std::cout << "Nodes max:" << *nodes << std::endl;
	//std::cout << "Nodes Final:" << *(nodes + 1) << std::endl;
	//std::cout << "===================================" << std::endl;

	//std::cout << "File name:" << file_name << std::endl;
	//auto dd3 = std::make_unique<dd::Package<>>(3 * n);
	//start_t = clock();
	//nodes = Simulate_with_partition2(path2, file_name, dd3);
	//finish_t = clock();
	//time_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;

	//std::cout << "Time:" << time_t << std::endl;
	//std::cout << "Nodes max:" << *nodes << std::endl;
	//std::cout << "Nodes Final:" << *(nodes + 1) << std::endl;
	//std::cout << "===================================" << std::endl;

	//save_data();
	system("pause");
	return 0;
}

int save_data() {

	std::ofstream  ofile;

	string path2 = "Benchmarks3/";
	std::string file_list_txt = "test2.txt";
	std::ifstream  file_list2;
	std::string line2;
	clock_t   start2, finish2;
	double time2;
	int* nodes2;


	ofile.open("data.csv", ios::app);
	ofile << "Simulate_with_tdd" << endl;
	ofile << "benchmarks" << "," << "time" << "," << "node max" << "," << "node final" << endl;
	file_list2.open(file_list_txt);
	while (std::getline(file_list2, line2)) {
		std::cout << "file name:" << line2 << std::endl;
		int n = get_qubits_num(path2 + line2);
		auto dd = std::make_unique<dd::Package<>>(3 * n);
		start2 = clock();
		nodes2 = Simulate_with_tdd(path2, line2, dd);
		finish2 = clock();
		time2 = (double)(finish2 - start2) / CLOCKS_PER_SEC;
		std::cout << "time:" << time2 << std::endl;
		std::cout << "nodes max:" << *nodes2 << std::endl;
		std::cout << "nodes final:" << *(nodes2 + 1) << std::endl;
		ofile << line2 << "," << time2 << "," << *nodes2 << "," << *(nodes2 + 1) << endl;
	}
	file_list2.close();
	ofile.close();


	ofile.open("data.csv", ios::app);
	ofile << "Simulate_with_partition1" << endl;
	ofile << "benchmarks" << "," << "time" << "," << "node max" << "," << "node final" << endl;
	file_list2.open(file_list_txt);
	while (std::getline(file_list2, line2)) {
		std::cout << "file name:" << line2 << std::endl;
		int n = get_qubits_num(path2 + line2);
		auto dd = std::make_unique<dd::Package<>>(3 * n);
		start2 = clock();
		nodes2 = Simulate_with_partition1(path2, line2, dd);
		finish2 = clock();
		time2 = (double)(finish2 - start2) / CLOCKS_PER_SEC;
		std::cout << "time:" << time2 << std::endl;
		std::cout << "nodes max:" << *nodes2 << std::endl;
		std::cout << "nodes final:" << *(nodes2 + 1) << std::endl;
		ofile << line2 << "," << time2 << "," << *nodes2 << "," << *(nodes2 + 1) << endl;
	}
	file_list2.close();
	ofile.close();

	ofile.open("data.csv", ios::app);
	ofile << "Simulate_with_partition2" << endl;
	ofile << "benchmarks" << "," << "time" << "," << "node max" << "," << "node final" << endl;
	file_list2.open(file_list_txt);
	while (std::getline(file_list2, line2)) {
		std::cout << "file name:" << line2 << std::endl;
		int n = get_qubits_num(path2 + line2);
		auto dd = std::make_unique<dd::Package<>>(3 * n);
		start2 = clock();
		nodes2 = Simulate_with_partition2(path2, line2, dd);
		finish2 = clock();
		time2 = (double)(finish2 - start2) / CLOCKS_PER_SEC;
		std::cout << "time:" << time2 << std::endl;
		std::cout << "nodes max:" << *nodes2 << std::endl;
		std::cout << "nodes final:" << *(nodes2 + 1) << std::endl;
		ofile << line2 << "," << time2 << "," << *nodes2 << "," << *(nodes2 + 1) << endl;
	}
	file_list2.close();
	ofile.close();

	system("pause");
	return 0;
}

// char* contractCircuit(char* circuit_p, int qubits, char* plan_p) {

// 	std::string plan(plan_p);
// 	std::string circuit(circuit_p);

// 	std::vector<std::tuple<int, int>> actualPlan = get_actual_plan_from_string(plan);

// 	int n = get_qubits_num_from_circuit(circuit);
// 	int gates = get_gates_num_from_circuit(circuit);
// 	auto dd = std::make_unique<dd::Package<>>(2 * gates);

// 	auto start = std::chrono::system_clock::now();
// 	dd::TDD res = plannedContractionOnCircuit(circuit, actualPlan, dd);
// 	auto end = std::chrono::system_clock::now();

// 	double contTime = (end-start).count();
// 	bool resIsIdentity = dd->isTDDIdentity(res, false, n);

// 	return (resIsIdentity + ";" + std::to_string(contTime)).data();
// }

const char* contractCircuit(char* circuit_p, int qubits, char* plan_p, char* res_filename_p, bool debugging) {
  
	to_test = debugging;
	std::string plan(plan_p);
	std::string circuit(circuit_p);
	std::string res_filename(res_filename_p);

	std::vector<std::tuple<int, int>> actualPlan = get_actual_plan_from_string(plan);

	//int n = get_qubits_num_from_circuit(circuit);
	int gates = get_gates_num_from_circuit(circuit);
	auto dd = std::make_unique<dd::Package<>>(2 * gates);

	std::tuple<dd::TDD, long> res = plannedContractionOnCircuit(circuit, actualPlan, dd, res_filename, debugging);
    
	dd::export2Dot(std::get<0>(res).e, res_filename);

	bool resIsIdentity = dd->isTDDIdentity(std::get<0>(res), false, qubits);

	//return (resIsIdentity + ";" + std::to_string(contTime)).data();
    

	return ((resIsIdentity ? "true" : "false") + std::string("; ") + std::to_string(std::get<1>(res))).data();
}



   




extern "C" {
	const char* pyContractCircuit(char* circuit_p, int qubits, char* plan_p, char* res_filename, bool debugging) {
		return contractCircuit(circuit_p, qubits, plan_p, res_filename, debugging);
	}

	int testerFunc(int num) {
		return num + 1;
	}

}