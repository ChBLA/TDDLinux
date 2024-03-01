// TDDLinux.cpp : Defines the entry point for the application.
//

//#include "TDDLinux.h"
#include <string>
#include <iostream>


using namespace std;

//int save_data();


//int main3(int argc, char* argv[])
//{
//	xt::xarray<double> arr1
//	{ {1.0, 2.0, 3.0},
//	  { 2.0, 5.0, 7.0 },
//	  { 2.0, 5.0, 7.0 } };
//
//	xt::xarray<dd::ComplexValue> arr2
//	{ {1, 2}, { 3,4 }, {5,6} };
//
//	xt::xarray<double> arr3
//	{ { {1.0, 2.0, 3.0},
//		{ 2.0, 5.0, 7.0 },
//		{ 2.0, 5.0, 7.0 } },
//		{ {1.0, 2.0, 3.0},
//		{ 2.0, 5.0, 7.0 },
//		{ 2.0, 5.0, 7.0 } } };
//
//	std::cout << xt::view(arr3, xt::all(), 0, 1) << std::endl;
//
//	auto shape = arr2.shape();
//
//
//	std::cout << arr2.size() << std::endl;
//
//	auto dd = std::make_unique<dd::Package<>>(100);
//
//	/*xt::xarray<int> U = { {{{1, 0}, {0, 1}}, {{0, 0}, {0, 0}}}, {{{0, 0}, {0, 0}}, {{0, 1}, {1, 0}}} };
//	dd::ComplexValue v = { 1,0 };*/
//
//	//xt::xarray<dd::ComplexValue> K = { {{{1, 0}, {0, 1}}, {{0, 0}, {0, 0}}}, {{{0, 0}, {0, 0}}, {{0, 1}, {1, 0}}} };
//	//xt::xarray<dd::ComplexValue> K = { {{1, 0}, {0, 1}}, {{0, 0}, {0, 0}} };
//	//xt::xarray<dd::ComplexValue> K = { {1, 0}, {0, 1} };
//	xt::xarray<dd::ComplexValue> K1 = { { {1, 0}, {1, 0} }, { {1, 0}, {-1, 0} } }; // works
//	xt::xarray<dd::ComplexValue> K2 = { { {1, 0}, {1, 0} }, { {1, 0}, {-1, 0} } }; // works
//	//xt::xarray<dd::ComplexValue> K = { {{1, 0}, {0, 1}}, {{1, 0}, {0, 0}} };
//	//xt::xarray<dd::ComplexValue> K = (xt::xarray<dd::ComplexValue>) U;
//	//dd::Tensor ts = { K,{{"a", 1},{"b", 2},{"c", 3}},"abc" };
//	//dd::Tensor ts = { K,{{"a", 1},{"b", 2}},"abc" };
//	//dd::Tensor ts = { K,{{"a", 1},{"b", 2}},"abc" };
//	//dd::Tensor ts = { K,{{"a", 1},{"b", 2}},"abc" }; // works
//	dd::Tensor ts1 = { K1,{{"a", 1}, {"c", 2}},"a" };
//	dd::Tensor ts2 = { K2,{{"c", 2}, {"b", 3}},"b" };
//	dd->varOrder = { {"a", 1}, {"c", 2}, {"b", 3} };
//	auto tdd1 = dd->Tensor_2_TDD(ts1);
//	auto tdd2 = dd->Tensor_2_TDD(ts2);
//
//	auto res = dd->cont(tdd1, tdd2);
//
//	bool isIden = dd->isTDDIdentity(res, false, 1);
//
//	dd::export2Dot(tdd1.e, "tdd1");
//	dd::export2Dot(tdd2.e, "tdd2");
//	dd::export2Dot(res.e, "tddRes");
//
//	return 0;
//}

int main() {
	string a = "a";
	std::cout << a << std::endl;
	return 0;
}

//int main7() {
//	string path2 = "../../../Benchmarks/";
//	string file_name = "test.qasm";
//	int n = get_qubits_num(path2 + file_name);
//	auto dd = std::make_unique<dd::Package<>>(3 * n);
//	std::cout << "File name:" << file_name << std::endl;
//	std::vector<std::tuple<int, int>> plan = getDefaultPlan(get_gates_num(path2 + file_name));
//	dd::TDD res = plannedContractionOnCircuit(path2, plan, file_name, dd);
//	std::cout << "Done" << std::endl;
//
//}
//
//int main2() {
//
//	//qc::QuantumComputation qc1{};
//
//	//string path = "Benchmarks/test.qasm";
//	//qc1.import(path, qc::Format::OpenQASM);
//	//const qc::MatrixDD dd1 = buildFunctionality(&qc1, dd);
//	//dd->printInformation();
//	//serialize(dd1, "output.ser");
//
//
//	string path2 = "Benchmarks/";
//
//	string file_name = "test.qasm";
//	int* nodes;
//	int n = get_qubits_num(path2 + file_name);
//	auto dd = std::make_unique<dd::Package<>>(3 * n);
//	clock_t   start_t, finish_t;
//	double time_t;
//	std::cout << "File name:" << file_name << std::endl;
//	start_t = clock();
//	nodes = Simulate_with_tdd(path2, file_name, dd);
//	finish_t = clock();
//	time_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
//	std::cout << "Time:" << time_t << std::endl;
//	std::cout << "Nodes max:" << *nodes << std::endl;
//	std::cout << "Nodes Final:" << *(nodes + 1) << std::endl;
//	std::cout << "===================================" << std::endl;
//
//
//	//std::cout << "File name:" << file_name << std::endl;
//	//auto dd2 = std::make_unique<dd::Package<>>(3 * n);
//	//start_t = clock();
//	//nodes = Simulate_with_partition1(path2, file_name,dd2);
//	//finish_t = clock();
//	//time_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
//
//	//std::cout << "Time:" << time_t << std::endl;
//	//std::cout << "Nodes max:" << *nodes << std::endl;
//	//std::cout << "Nodes Final:" << *(nodes + 1) << std::endl;
//	//std::cout << "===================================" << std::endl;
//
//	//std::cout << "File name:" << file_name << std::endl;
//	//auto dd3 = std::make_unique<dd::Package<>>(3 * n);
//	//start_t = clock();
//	//nodes = Simulate_with_partition2(path2, file_name, dd3);
//	//finish_t = clock();
//	//time_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
//
//	//std::cout << "Time:" << time_t << std::endl;
//	//std::cout << "Nodes max:" << *nodes << std::endl;
//	//std::cout << "Nodes Final:" << *(nodes + 1) << std::endl;
//	//std::cout << "===================================" << std::endl;
//
//	//save_data();
//	system("pause");
//	return 0;
//}
//
//int save_data() {
//
//	std::ofstream  ofile;
//
//	string path2 = "Benchmarks3/";
//	std::string file_list_txt = "test2.txt";
//	std::ifstream  file_list2;
//	std::string line2;
//	clock_t   start2, finish2;
//	double time2;
//	int* nodes2;
//
//
//	ofile.open("data.csv", ios::app);
//	ofile << "Simulate_with_tdd" << endl;
//	ofile << "benchmarks" << "," << "time" << "," << "node max" << "," << "node final" << endl;
//	file_list2.open(file_list_txt);
//	while (std::getline(file_list2, line2)) {
//		std::cout << "file name:" << line2 << std::endl;
//		int n = get_qubits_num(path2 + line2);
//		auto dd = std::make_unique<dd::Package<>>(3 * n);
//		start2 = clock();
//		nodes2 = Simulate_with_tdd(path2, line2, dd);
//		finish2 = clock();
//		time2 = (double)(finish2 - start2) / CLOCKS_PER_SEC;
//		std::cout << "time:" << time2 << std::endl;
//		std::cout << "nodes max:" << *nodes2 << std::endl;
//		std::cout << "nodes final:" << *(nodes2 + 1) << std::endl;
//		ofile << line2 << "," << time2 << "," << *nodes2 << "," << *(nodes2 + 1) << endl;
//	}
//	file_list2.close();
//	ofile.close();
//
//
//	ofile.open("data.csv", ios::app);
//	ofile << "Simulate_with_partition1" << endl;
//	ofile << "benchmarks" << "," << "time" << "," << "node max" << "," << "node final" << endl;
//	file_list2.open(file_list_txt);
//	while (std::getline(file_list2, line2)) {
//		std::cout << "file name:" << line2 << std::endl;
//		int n = get_qubits_num(path2 + line2);
//		auto dd = std::make_unique<dd::Package<>>(3 * n);
//		start2 = clock();
//		nodes2 = Simulate_with_partition1(path2, line2, dd);
//		finish2 = clock();
//		time2 = (double)(finish2 - start2) / CLOCKS_PER_SEC;
//		std::cout << "time:" << time2 << std::endl;
//		std::cout << "nodes max:" << *nodes2 << std::endl;
//		std::cout << "nodes final:" << *(nodes2 + 1) << std::endl;
//		ofile << line2 << "," << time2 << "," << *nodes2 << "," << *(nodes2 + 1) << endl;
//	}
//	file_list2.close();
//	ofile.close();
//
//	ofile.open("data.csv", ios::app);
//	ofile << "Simulate_with_partition2" << endl;
//	ofile << "benchmarks" << "," << "time" << "," << "node max" << "," << "node final" << endl;
//	file_list2.open(file_list_txt);
//	while (std::getline(file_list2, line2)) {
//		std::cout << "file name:" << line2 << std::endl;
//		int n = get_qubits_num(path2 + line2);
//		auto dd = std::make_unique<dd::Package<>>(3 * n);
//		start2 = clock();
//		nodes2 = Simulate_with_partition2(path2, line2, dd);
//		finish2 = clock();
//		time2 = (double)(finish2 - start2) / CLOCKS_PER_SEC;
//		std::cout << "time:" << time2 << std::endl;
//		std::cout << "nodes max:" << *nodes2 << std::endl;
//		std::cout << "nodes final:" << *(nodes2 + 1) << std::endl;
//		ofile << line2 << "," << time2 << "," << *nodes2 << "," << *(nodes2 + 1) << endl;
//	}
//	file_list2.close();
//	ofile.close();
//
//	system("pause");
//	return 0;
//}