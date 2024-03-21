#pragma once

//#include "DDpackage.h"
//#include "DDcomplex.h"

#include "dd/Package.hpp"
#include "dd/Export.hpp"
#include "nlohmann/json.hpp"

#include <unordered_set>
#include <vector>
#include <array>
#include <bitset>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <regex>
#include"time.h"
#include <math.h>
#include <chrono>
#include <ctime>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <filesystem>

using namespace std;
using json = nlohmann::json;
//auto dd = std::make_unique<dd::Package<>>(100);

constexpr long double PI = 3.14159265358979323846264338327950288419716939937510L;

bool release = true;
bool get_max_node = false;
bool to_test = false;

struct gate {
	std::string name;
	short int qubits[2];
	//std::vector<dd::Index> index_set;
};

int qubits_num = 0;
int gates_num = 0;
clock_t   start, finish;
void print_index_set(std::vector<dd::Index> index_set) {
	for (int k = 0; k < index_set.size(); k++) {
		std::cout << "(" << index_set[k].key << ", " << index_set[k].idx << ") ,";

	}
	std::cout << std::endl;
}
//是为了从qasm文件的行条目中提取出门的name和qubit
vector<std::string> split(const std::string& s, const std::string& seperator) {
	vector<std::string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()) {
		//找到字符串中首个不等于分隔符的字母；
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

		//找到又一个分隔符，将两个分隔符之间的字符串取出；
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


float match_a_string(string s) {
	smatch result;
	regex pattern("(-?\\d+.\\d+)");
	regex pattern2("(-?\\d+.\\d+)\\*?pi/(\\d+)");
	regex pattern3("(-?\\d+.\\d+)\\*?pi");
	regex pattern4("pi/(\\d+)");
	regex pattern5("(\\d+)");
	regex pattern6("-pi/(\\d+)");
	if (regex_match(s, result, pattern)) {
		//cout << result[1] << endl;
		return stof(result[1]);
	}
	else if (regex_match(s, result, pattern2)) {
		//cout << result[1] << ',' << result[2] << endl;
		return stof(result[1]) * PI / stof(result[2]);
	}
	else if (regex_match(s, result, pattern3)) {
		//cout << result[1] << endl;
		return stof(result[1]) * PI;
	}
	else if (regex_match(s, result, pattern4)) {
		//cout << result[1] << endl;
		return PI / stof(result[1]);
	}
	else if (regex_match(s, result, pattern5)) {
		//cout << result[1] << endl;
		return stof(result[1]);
	}
	else if (regex_match(s, result, pattern6)) {
		//cout << result[1] << endl;
		return -PI / stof(result[1]);
	}
	std::cout << s << endl;
	std::cout << "Not Macth" << endl;
	return 0.0;
}

//导入一个qasm文件
std::map<int, gate> import_circuit(std::string  file_name) {

	qubits_num = 0;
	gates_num = 0;

	std::map<int, gate> gate_set;

	std::ifstream  infile;

	infile.open(file_name);

	std::string line;
	std::getline(infile, line);
	std::getline(infile, line);
	//std::getline(infile, line);
	std::getline(infile, line);
	while (std::getline(infile, line))
	{
		gate temp_gate;

		vector<std::string> g = split(line, " ");
		smatch result;

		temp_gate.name = g[0];

		if (g[0] == "cx") {
			regex pattern("q\\[(\\d+)\\], ?q\\[(\\d+)\\];");
			if (regex_match(g[1], result, pattern))
			{
				if (stoi(result[1]) > qubits_num) {
					qubits_num = stoi(result[1]);
				}
				if (stoi(result[2]) > qubits_num) {
					qubits_num = stoi(result[2]);
				}
				temp_gate.qubits[0] = stoi(result[1]);
				temp_gate.qubits[1] = stoi(result[2]);
			}

		}
		else {
			regex pattern("q\\[(\\d+)\\];");
			if (regex_match(g[1], result, pattern))
			{
				if (stoi(result[1]) > qubits_num) {
					qubits_num = stoi(result[1]);
				}
				temp_gate.qubits[0] = stoi(result[1]);
			}
		}

		gate_set[gates_num] = temp_gate;
		gates_num++;
	}
	infile.close();
	qubits_num += 1;
	return gate_set;
}

std::map<int, gate> import_circuit_from_string(std::string circuit) {

	qubits_num = 0;
	gates_num = 0;

	std::map<int, gate> gate_set;

	std::stringstream infile(circuit);

	std::string line;
	std::getline(infile, line);
	std::getline(infile, line);
	//std::getline(infile, line);
	std::getline(infile, line);
	while (std::getline(infile, line))
	{
		gate temp_gate;

		vector<std::string> g = split(line, " ");
		smatch result;

		temp_gate.name = g[0];

		if (g[0] == "cx" || g[0] == "cy" || g[0] == "cz" || g[0] == "cnot") {
			regex pattern("q\\[(\\d+)\\], ?q\\[(\\d+)\\];");
			if (regex_match(g[1], result, pattern))
			{
				if (stoi(result[1]) > qubits_num) {
					qubits_num = stoi(result[1]);
				}
				if (stoi(result[2]) > qubits_num) {
					qubits_num = stoi(result[2]);
				}
				temp_gate.qubits[0] = stoi(result[1]);
				temp_gate.qubits[1] = stoi(result[2]);
			}

		}
		else {
			regex pattern("q\\[(\\d+)\\];");
			if (regex_match(g[1], result, pattern))
			{
				if (stoi(result[1]) > qubits_num) {
					qubits_num = stoi(result[1]);
				}
				temp_gate.qubits[0] = stoi(result[1]);
			}
		}

		gate_set[gates_num] = temp_gate;
		gates_num++;
	}
	
	qubits_num += 1;
	return gate_set;
}

std::map<int, std::vector<dd::Index>> get_index(std::map<int, gate> gate_set, std::map<std::string, int> var) {

	std::map<int, std::vector<dd::Index>> Index_set;

	std::map<std::string, short> hyper_idx;


	for (const auto& pair : var) {
		hyper_idx[pair.first] = 0;
	}



	int* qubit_idx = new   int[qubits_num]();


	for (int k = 0; k < gates_num; k++)
	{
		//std::cout << k << std::endl;
		std::string nam = gate_set[k].name;
		//std::cout << nam << std::endl;
		//std::cout << gate_set[k].qubits[0]<<"    "<<gate_set[k].qubits[1] << endl;
		if (nam == "cx" || nam == "cy" || nam == "cz" || nam == "cnot") {
			int con_q = gate_set[k].qubits[0];
			int tar_q = gate_set[k].qubits[1];
			std::string cont_idx1 = "x";
			cont_idx1 += to_string(con_q);
			cont_idx1 += "_";
			cont_idx1 += to_string(qubit_idx[con_q]);
			qubit_idx[con_q] += 1;
			std::string cont_idx2 = "x";
			cont_idx2 += to_string(con_q);
			cont_idx2 += "_";
			cont_idx2 += to_string(qubit_idx[con_q]);
			std::string targ_idx1 = "x";
			targ_idx1 += to_string(tar_q);
			targ_idx1 += "_";
			targ_idx1 += to_string(qubit_idx[tar_q]);
			qubit_idx[tar_q] += 1;
			std::string targ_idx2 = "x";
			targ_idx2 += to_string(tar_q);
			targ_idx2 += "_";
			targ_idx2 += to_string(qubit_idx[tar_q]);
			Index_set[k] = { {cont_idx1,hyper_idx[cont_idx1]},{cont_idx2,hyper_idx[cont_idx2]},{targ_idx1,hyper_idx[targ_idx1]},{targ_idx2,hyper_idx[targ_idx2]} };
			//std::cout << cont_idx<<" " << hyper_idx[cont_idx] << " " << cont_idx << " " << hyper_idx[cont_idx] + 1 << " " << cont_idx << " " << hyper_idx[cont_idx] + 2 << " " << targ_idx1 << " " << hyper_idx[targ_idx1] << " " << targ_idx2 << " " <<hyper_idx[targ_idx2] << " " << std::endl;
			//hyper_idx[cont_idx] += 2;

		}
		else {
			int tar_q = gate_set[k].qubits[0];
			std::string targ_idx1 = "x";
			std::string targ_idx2 = "x";

			targ_idx1 += to_string(tar_q);
			targ_idx1 += "_";
			targ_idx1 += to_string(qubit_idx[tar_q]);
			qubit_idx[tar_q] += 1;
			targ_idx2 += to_string(tar_q);
			targ_idx2 += "_";
			targ_idx2 += to_string(qubit_idx[tar_q]);
			Index_set[k] = { {targ_idx1,hyper_idx[targ_idx1]},{targ_idx2,hyper_idx[targ_idx2]} };
			if (false && (nam == "x" || nam == "h" || nam == "z" || nam == "s" || nam == "sdg" || nam == "t" || nam == "tdg" || (nam[0] == 'u' && nam[1] == '1') || (nam[0] == 'r' && nam[1] == 'z') || (nam[0] == 'r' && nam[1] == 'y'))) {
				Index_set[k] = { {targ_idx1,hyper_idx[targ_idx1]},{targ_idx1,hyper_idx[targ_idx1] + 1} };
				qubit_idx[tar_q] -= 1;
				hyper_idx[targ_idx1] += 1;
			}
			else {
				Index_set[k] = { {targ_idx1,hyper_idx[targ_idx1]},{targ_idx2,hyper_idx[targ_idx2]} };
			}
		}
		//std::cout << k << " ";
		//print_index_set(Index_set[k]);
	}
	return Index_set;
}


std::map<int, map<int, std::vector<int>>>  cir_partition1(std::map<int, gate> gate_set, int cx_cut_max) {
	std::map<int, map<int, std::vector<int>>>   par;
	int cx_cut = 0;
	int block = 0;
	for (int k = 0; k < gates_num; k++)
	{
		std::string nam = gate_set[k].name;
		if (nam != "cx") {
			if (gate_set[k].qubits[0] <= qubits_num / 2) {
				par[block][0].push_back(k);
			}
			else {
				par[block][1].push_back(k);
			}
		}
		else {
			if (gate_set[k].qubits[0] <= qubits_num / 2 && gate_set[k].qubits[1] <= qubits_num / 2) {
				par[block][0].push_back(k);
			}
			else if (gate_set[k].qubits[0] > qubits_num / 2 && gate_set[k].qubits[1] > qubits_num / 2) {
				par[block][1].push_back(k);
			}
			else {
				if (cx_cut <= cx_cut_max) {
					if (gate_set[k].qubits[1] > qubits_num / 2) {
						par[block][1].push_back(k);
					}
					else {
						par[block][0].push_back(k);
					}
					cx_cut += 1;
				}
				else {
					block += 1;
					cx_cut = 1;
					if (gate_set[k].qubits[1] > qubits_num / 2) {
						par[block][1].push_back(k);
					}
					else {
						par[block][0].push_back(k);
					}
				}
			}
		}
	}

	//for (int k = 0; k < par.size(); k++) {
	//	for (int k1 = 0; k1 < par[k][0].size(); k1++) {
	//		cout << par[k][0][k1] << "  ";
	//	}
	//	cout << endl;
	//	for (int k1 = 0; k1 < par[k][1].size(); k1++) {
	//		cout << par[k][1][k1] << "  ";
	//	}
	//	cout << endl;
	//	cout << "--------" << endl;
	//}


	return par;
}

int min(int a, int b) {
	if (a <= b) {

		return a;
	}
	else {
		return b;
	}
}
int max(int a, int b) {
	if (a >= b) {

		return a;
	}
	else {
		return b;
	}
}

std::map<int, map<int, std::vector<int>>>  cir_partition2(std::map<int, gate> gate_set, int cx_cut_max, int c_part_width) {
	std::map<int, map<int, std::vector<int>>>   par;
	int cx_cut = 0;
	int block = 0;
	int c_part_min = qubits_num / 2;
	int c_part_max = qubits_num / 2;
	for (int k = 0; k < gates_num; k++)
	{
		std::string nam = gate_set[k].name;

		if (cx_cut <= cx_cut_max) {

			if (nam != "cx") {
				if (gate_set[k].qubits[0] <= qubits_num / 2) {
					par[block][0].push_back(k);
				}
				else {
					par[block][1].push_back(k);
				}
			}
			else {
				if (gate_set[k].qubits[0] <= qubits_num / 2 && gate_set[k].qubits[1] <= qubits_num / 2) {
					par[block][0].push_back(k);
				}
				else if (gate_set[k].qubits[0] > qubits_num / 2 && gate_set[k].qubits[1] > qubits_num / 2) {
					par[block][1].push_back(k);
				}
				else {
					if (gate_set[k].qubits[1] > qubits_num / 2) {
						par[block][1].push_back(k);
					}
					else {
						par[block][0].push_back(k);
					}
					cx_cut += 1;
				}
			}
		}
		else {
			if (nam != "cx") {
				if (gate_set[k].qubits[0] < c_part_min) {
					par[block][0].push_back(k);
				}
				else if (gate_set[k].qubits[0] > c_part_max) {
					par[block][1].push_back(k);
				}
				else {
					par[block][2].push_back(k);
				}
			}
			else if (gate_set[k].qubits[0] >= c_part_min && gate_set[k].qubits[0] <= c_part_max && gate_set[k].qubits[1] >= c_part_min && gate_set[k].qubits[1] <= c_part_max) {
				par[block][2].push_back(k);
			}
			else if (gate_set[k].qubits[0] < c_part_min && gate_set[k].qubits[1] < c_part_min)
			{
				par[block][0].push_back(k);
			}
			else if (gate_set[k].qubits[0] > c_part_max && gate_set[k].qubits[1] > c_part_max)
			{
				par[block][1].push_back(k);
			}
			else {
				int temp_c_min = min(c_part_min, min(gate_set[k].qubits[0], gate_set[k].qubits[1]));
				int temp_c_max = max(c_part_max, max(gate_set[k].qubits[0], gate_set[k].qubits[1]));
				if ((temp_c_max - temp_c_min) > c_part_width) {
					block += 1;
					cx_cut = 0;
					c_part_min = qubits_num / 2;
					c_part_max = qubits_num / 2;
					if (gate_set[k].qubits[0] <= qubits_num / 2 && gate_set[k].qubits[1] <= qubits_num / 2) {
						par[block][0].push_back(k);
					}
					else if (gate_set[k].qubits[0] > qubits_num / 2 && gate_set[k].qubits[1] > qubits_num / 2) {
						par[block][1].push_back(k);
					}
					else {
						if (gate_set[k].qubits[1] > qubits_num / 2) {
							par[block][1].push_back(k);
						}
						else {
							par[block][0].push_back(k);
						}
						cx_cut += 1;
					}
				}
				else {
					par[block][2].push_back(k);
					c_part_min = temp_c_min;
					c_part_max = temp_c_max;
				}
			}
		}
	}

	//for (int k = 0; k < 1; k++) {
	//for (int k1 = 0; k1 < par[k][0].size(); k1++) {
	//	cout << par[k][0][k1] << "  ";
	//}
	//cout << endl;
	//for (int k1 = 0; k1 < par[k][1].size(); k1++) {
	//	cout << par[k][1][k1] << "  ";
	//}
	//cout << endl;
	//for (int k1 = 0; k1 < par[k][2].size(); k1++) {
	//	cout << par[k][2][k1] << "  ";
	//}
	//cout << endl;
	//cout << "--------" << endl;
	//}
	return par;
}


dd::TDD apply(dd::TDD tdd, std::string nam, std::vector<dd::Index> index_set, std::unique_ptr<dd::Package<>>& dd) {

	//std::cout << nam << std::endl;

	std::map<std::string, int> gate_type;
	gate_type["x"] = 1;
	gate_type["y"] = 2;
	gate_type["z"] = 3;
	gate_type["h"] = 4;
	gate_type["s"] = 5;
	gate_type["sdg"] = 6;
	gate_type["t"] = 7;
	gate_type["tdg"] = 8;

	dd::TDD temp_tdd;

	if (nam == "cx") {
		temp_tdd = dd->cnot_2_TDD(index_set, 1);
	}
	else {
		switch (gate_type[nam]) {
		case 1:
			temp_tdd = dd->Matrix2TDD(dd::Xmat, index_set);
			break;
		case 2:
			temp_tdd = dd->Matrix2TDD(dd::Ymat, index_set);

			//std::cout << temp_tdd.e.w << " " << int(temp_tdd.e.p->v) << std::endl;
			//std::cout << temp_tdd.e.p->e[0].w << " " << int(temp_tdd.e.p->e[0].p->v) << std::endl;
			//std::cout << temp_tdd.e.p->e[1].w << " " << int(temp_tdd.e.p->e[1].p->v) << std::endl;

			break;
		case 3:
			temp_tdd = dd->diag_matrix_2_TDD(dd::Zmat, index_set);
			break;
		case 4:
			temp_tdd = dd->Matrix2TDD(dd::Hmat, index_set);
			break;
		case 5:
			temp_tdd = dd->diag_matrix_2_TDD(dd::Smat, index_set);
			break;
		case 6:
			temp_tdd = dd->diag_matrix_2_TDD(dd::Sdagmat, index_set);
			break;
		case 7:
			temp_tdd = dd->diag_matrix_2_TDD(dd::Tmat, index_set);
			break;
		case 8:
			temp_tdd = dd->diag_matrix_2_TDD(dd::Tdagmat, index_set);
			break;
		default:
			if (nam[0] == 'r' and nam[1] == 'z') {
				regex pattern("rz\\((-?\\d.\\d+)\\)");
				smatch result;
				regex_match(nam, result, pattern);
				float theta = stof(result[1]);
				//dd::GateMatrix Rzmat = { { 1, 0 }, { 0, 0 } , { 0, 0 }, { cos(theta), sin(theta) } };
				temp_tdd = dd->diag_matrix_2_TDD(dd::Phasemat(theta), index_set);
				break;
			}
			if (nam[0] == 'u' and nam[1] == '1') {
				//regex pattern("u1\\((-?\\d.\\d+)\\)");
				//smatch result;
				//regex_match(nam, result, pattern);
				//float theta = stof(result[1]);

				regex para(".*?\\((.*?)\\)");
				smatch result;
				regex_match(nam, result, para);
				float theta = match_a_string(result[1]);

				//dd::GateMatrix  U1mat = { { 1, 0 }, { 0, 0 } , { 0, 0 }, { cos(theta), sin(theta) }  };

				temp_tdd = dd->diag_matrix_2_TDD(dd::Phasemat(theta), index_set);
				break;
			}
			if (nam[0] == 'u' and nam[1] == '3') {
				//regex pattern("u3\\((-?\\d.\\d+), ?(-?\\d.\\d+), ?(-?\\d.\\d+)\\)");
				//smatch result;
				//regex_match(nam, result, pattern);
				//float theta = stof(result[1]);
				//float phi = stof(result[2]);
				//float lambda = stof(result[3]);

				regex para(".*?\\((.*?)\\)");
				smatch result;
				regex_match(nam, result, para);
				vector<string> para2 = split(result[1], ",");
				float theta = match_a_string(para2[0]);
				float phi = match_a_string(para2[1]);
				float lambda = match_a_string(para2[2]);
				//dd::GateMatrix  U3mat = { { cos(theta / 2), 0 }, { -cos(lambda) * sin(theta / 2),-sin(lambda) * sin(theta / 2)} , { cos(phi) * sin(theta / 2),sin(phi) * sin(theta / 2) }, { cos(lambda + phi) * cos(theta / 2),sin(lambda + phi) * cos(theta / 2) }  };
				temp_tdd = dd->Matrix2TDD(dd::U3mat(lambda, phi, theta), index_set);
				break;
			}
		}
	}


	if (release) {
		auto tmp = dd->cont(tdd, temp_tdd);
		dd->incRef(tmp.e);
		dd->decRef(tdd.e);
		tdd = tmp;
		dd->garbageCollect();
	}
	else {
		tdd = dd->cont(tdd, temp_tdd);
	}



	return tdd;
}


dd::TDD gateToTDD(std::string nam, std::vector<dd::Index> index_set, std::unique_ptr<dd::Package<>>& dd) {

	//std::cout << nam << std::endl;

	std::map<std::string, int> gate_type;
	gate_type["x"] = 1;
	gate_type["y"] = 2;
	gate_type["z"] = 3;
	gate_type["h"] = 4;
	gate_type["s"] = 5;
	gate_type["sdg"] = 6;
	gate_type["t"] = 7;
	gate_type["tdg"] = 8;
	gate_type["id"] = 9;

	dd::TDD temp_tdd;
	std::string gate_name = nam;
	std::vector<dd::fp> params = {};
	if (nam == "cx" || nam == "cnot") {
		//temp_tdd = dd->cnot_2_TDD(index_set, 1);
		temp_tdd = dd->cgate_2_TDD(index_set, "x");
	} else if (nam == "cy") {
		//temp_tdd = dd->cy_2_TDD(index_set, 1);
		temp_tdd = dd->cgate_2_TDD(index_set, "y");
	} else if (nam == "cz") {
		//temp_tdd = dd->cz_2_TDD(index_set, 1);
		temp_tdd = dd->cgate_2_TDD(index_set, "z");
	} else {
		switch (gate_type[nam]) {
		case 1:
			temp_tdd = dd->Matrix2TDD(dd::Xmat, index_set);
			break;
		case 2:
			temp_tdd = dd->Matrix2TDD(dd::Ymat, index_set);
			break;
		case 3:
			temp_tdd = dd->Matrix2TDD(dd::Zmat, index_set);
			break;
		case 4:
			temp_tdd = dd->Matrix2TDD(dd::Hmat, index_set);
			break;
		case 5:
			temp_tdd = dd->Matrix2TDD(dd::Smat, index_set);
			break;
		case 6:
			temp_tdd = dd->Matrix2TDD(dd::Sdagmat, index_set);
			break;
		case 7:
			temp_tdd = dd->Matrix2TDD(dd::Tmat, index_set);
			break;
		case 8:
			temp_tdd = dd->Matrix2TDD(dd::Tdagmat, index_set);
			break;
		case 9:
			temp_tdd = dd->Matrix2TDD(dd::Imat, index_set);
			break;
		default:
			gate_name = "";
			gate_name += nam[0];
			gate_name += nam[1];
			if (nam[0] == 'r' and nam[1] == 'z') {
				regex pattern("rz\\((-?\\d.\\d+)\\)");
				smatch result;
				regex_match(nam, result, pattern);
				float theta = stof(result[1]);
				params.push_back(theta);
				// dd::fp act_theta;
				// if (fabs(fabs(theta) - dd::PI) < 0.0000001) {
				// 	act_theta = dd::PI;
				// } else if (fabs(fabs(theta) - dd::PI_2) < 0.0000001) {
				// 	act_theta = dd::PI_2;
				// } else if (fabs(fabs(theta) - dd::PI_4) < 0.0000001) {
				// 	act_theta = dd::PI_4;
				// } else {
				// 	act_theta = fabs(theta);
				// }
				// temp_tdd = dd->diag_matrix_2_TDD(dd::RZmat(theta < 0.0 ? -act_theta : act_theta), index_set);
				if (to_test) {
					for (int i = 0; i < index_set.size(); i++) {
						printf("RZ index set before %d: %s\n", i, index_set[i].key.c_str());
					}
				}
				temp_tdd = dd->Matrix2TDD(dd::RZmat(theta), index_set);
				if (to_test) {
					for (int i = 0; i < temp_tdd.index_set.size(); i++) {
						printf("RZ index set after %d: %s\n", i, temp_tdd.index_set[i].key.c_str());
					}
				}
				break;
			}
			if (nam[0] == 'r' and nam[1] == 'y') {
				regex pattern("ry\\((-?\\d.\\d+)\\)");
				smatch result;
				regex_match(nam, result, pattern);
				float theta = stof(result[1]);
				params.push_back(theta);
				// dd::fp act_theta;
				// if (fabs(fabs(theta) - dd::PI) < 0.0000001) {
				// 	act_theta = dd::PI;
				// } else if (fabs(fabs(theta) - dd::PI_2) < 0.0000001) {
				// 	act_theta = dd::PI_2;
				// } else if (fabs(fabs(theta) - dd::PI_4) < 0.0000001) {
				// 	act_theta = dd::PI_4;
				// } else {
				// 	act_theta = fabs(theta);
				// }
				// temp_tdd = dd->diag_matrix_2_TDD(dd::RYmat(theta < 0.0 ? -act_theta : act_theta), index_set);
				temp_tdd = dd->Matrix2TDD(dd::RYmat(theta), index_set);
				break;
			}
			if (nam[0] == 'r' and nam[1] == 'x') {
				regex pattern("rx\\((-?\\d.\\d+)\\)");
				smatch result;
				regex_match(nam, result, pattern);
				float theta = stof(result[1]);
				params.push_back(theta);
				// dd::fp act_theta;
				// if (fabs(fabs(theta) - dd::PI) < 0.0000000000001) {
				// 	act_theta = dd::PI;
				// } else if (fabs(fabs(theta) - dd::PI_2) < 0.0000000000001) {
				// 	act_theta = dd::PI_2;
				// } else if (fabs(fabs(theta) - dd::PI_4) < 0.0000000000001) {
				// 	act_theta = dd::PI_4;
				// } else {
				// 	act_theta = fabs(theta);
				// }
				// temp_tdd = dd->diag_matrix_2_TDD(dd::RXmat(theta < 0.0 ? -act_theta : act_theta), index_set);
				temp_tdd = dd->Matrix2TDD(dd::RXmat(theta), index_set);
				break;
			}
			if (nam[0] == 'u' and nam[1] == '1') {
				//regex pattern("u1\\((-?\\d.\\d+)\\)");
				//smatch result;
				//regex_match(nam, result, pattern);
				//float theta = stof(result[1]);

				regex para(".*?\\((.*?)\\)");
				smatch result;
				regex_match(nam, result, para);
				float theta = match_a_string(result[1]);
				params.push_back(theta);

				temp_tdd = dd->Matrix2TDD(dd::Phasemat(theta), index_set);
				break;
			}
			if (nam[0] == 'u' and nam[1] == '3') {
				//regex pattern("u3\\((-?\\d.\\d+), ?(-?\\d.\\d+), ?(-?\\d.\\d+)\\)");
				//smatch result;
				//regex_match(nam, result, pattern);
				//float theta = stof(result[1]);
				//float phi = stof(result[2]);
				//float lambda = stof(result[3]);

				regex para(".*?\\((.*?)\\)");
				smatch result;
				regex_match(nam, result, para);
				vector<string> para2 = split(result[1], ",");
				if (to_test)
					printf("U3 (name = %s) num of parameters = %d\n", nam.c_str(), para2.size());
				float theta = match_a_string(para2[0]);
				float phi = match_a_string(para2[1]);
				float lambda = match_a_string(para2[2]);
				params.push_back(theta);
				params.push_back(phi);
				params.push_back(lambda);
				//dd::GateMatrix  U3mat = { { cos(theta / 2), 0 }, { -cos(lambda) * sin(theta / 2),-sin(lambda) * sin(theta / 2)} , { cos(phi) * sin(theta / 2),sin(phi) * sin(theta / 2) }, { cos(lambda + phi) * cos(theta / 2),sin(lambda + phi) * cos(theta / 2) }  };
				temp_tdd = dd->Matrix2TDD(dd::U3mat(lambda, phi, theta), index_set);
				break;
			}
		}
	}
	dd->incRef(temp_tdd.e);
	dd::GateDef temp_gate = {gate_name, params};
	temp_tdd.gates = {temp_gate};
	return temp_tdd;
}

dd::TDD applyTDDs(dd::TDD tdd1, dd::TDD tdd2, std::unique_ptr<dd::Package<>>& dd) {
	if (release) {
		auto tmp = dd->cont(tdd1, tdd2);
		dd->incRef(tmp.e);
		dd->decRef(tdd1.e);
		tdd1 = tmp;
		dd->garbageCollect();
	}
	else {
		tdd1 = dd->cont(tdd1, tdd2);
	}

	return tdd1;
}

dd::TDD applyTDDsWithJSON(dd::TDD tdd1, dd::TDD tdd2, std::unique_ptr<dd::Package<>>& dd, json& res_json) {
	std::vector<dd::GateDef> new_gates;
	new_gates.reserve(tdd1.gates.size() + tdd2.gates.size());
	new_gates.insert(new_gates.end(), tdd1.gates.begin(), tdd1.gates.end());
	new_gates.insert(new_gates.end(), tdd2.gates.begin(), tdd2.gates.end());

	json left;
	left["nodes"] = (int)dd->size(tdd1.e);
	left["gates"] = tdd1.gates;
	left["indices"] = tdd1.key_2_index;

	json right = {
		{"nodes", (int)dd->size(tdd2.e)},
		{"gates", tdd2.gates},
		{"indices", tdd2.key_2_index}
	};

	struct timeval start, end;
	long mtime, seconds, useconds;  

	if (release) {
		gettimeofday(&start, NULL);
		auto tmp = dd->cont(tdd1, tdd2);
		gettimeofday(&end, NULL);
		dd->incRef(tmp.e);
		dd->decRef(tdd1.e);
		tdd1 = tmp;
		dd->garbageCollect();
	}
	else {
		gettimeofday(&start, NULL);
		tdd1 = dd->cont(tdd1, tdd2);
		gettimeofday(&end, NULL);
	}

	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	//mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	mtime = useconds;

	tdd1.gates = new_gates;
	json result = {
		{"nodes", (int)dd->size(tdd1.e)},
		{"gates", tdd1.gates},
		{"indices", tdd1.key_2_index}
	};

	res_json["left"] = left;
	res_json["right"] = right;
	res_json["result"] = result;
	res_json["time"] = mtime;

	return tdd1;
}

std::map<std::string, int> get_var_order() {

	std::map<std::string, int> var;


	//设置变量顺序
	int order_num = 1000000;

	for (int k = qubits_num; k >= 0; k--) {
		string idx_nam;
		idx_nam = "y";
		idx_nam += to_string(k);
		var[idx_nam] = order_num;
		order_num -= 1;
		for (int k2 = gates_num; k2 >= 0; k2--) {
			idx_nam = "x";
			idx_nam += to_string(k);
			idx_nam += "_";
			idx_nam += to_string(k2);
			var[idx_nam] = order_num;
			order_num -= 1;
			//cout << idx_nam << endl;
		}
		idx_nam = "x";
		idx_nam += to_string(k);
		var[idx_nam] = order_num;
		order_num -= 1;
	}

	//var["-1"] = order_num;
	//int k = 0;
	//for (const auto& pair : var) {
	//	std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
	//	if (k > 20){
	//		break;
	//	}
	//	k++;
	//}
	return var;
}


int* Simulate_with_partition1(std::string path, std::string  file_name, std::unique_ptr<dd::Package<>>& dd) {

	std::map<int, gate> gate_set = import_circuit(path + file_name);

	int cx_cut_max = qubits_num / 2 + 1;
	std::map<int, map<int, std::vector<int>>>  par = cir_partition1(gate_set, cx_cut_max);

	int* nodes = new int[2];
	nodes[0] = 0;
	nodes[1] = 0;

	std::map<std::string, int> var;

	var = get_var_order();

	std::map<int, std::vector<dd::Index>> Index_set = get_index(gate_set, var);

	dd->varOrder = var;

	dd::TDD tdd = { dd::Edge<dd::mNode>::one ,{} };
	if (release) {
		dd->incRef(tdd.e);
	}
	start = clock();
	int node_num_max = 0;
	for (int k = 0; k < par.size(); k++) {
		//std::cout << "block:" << k << std::endl;
		dd::TDD temp_tdd1 = { dd::Edge<dd::mNode>::one ,{} };
		if (release) {
			dd->incRef(temp_tdd1.e);
		}
		for (int k1 = 0; k1 < par[k][0].size(); k1++) {
			int gate_idx = par[k][0][k1];
			string nam = gate_set[gate_idx].name;
			temp_tdd1 = apply(temp_tdd1, nam, Index_set[gate_idx], dd);
		}

		dd::TDD temp_tdd2 = { dd::Edge<dd::mNode>::one ,{} };
		if (release) {
			dd->incRef(temp_tdd2.e);
		}
		for (int k1 = 0; k1 < par[k][1].size(); k1++) {
			int gate_idx = par[k][1][k1];
			string nam = gate_set[gate_idx].name;
			temp_tdd2 = apply(temp_tdd2, nam, Index_set[gate_idx], dd);
		}
		if (release) {
			auto temp_tdd = dd->cont(temp_tdd1, temp_tdd2);
			dd->decRef(temp_tdd1.e);
			dd->decRef(temp_tdd2.e);
			auto tmp = dd->cont(tdd, temp_tdd);
			dd->incRef(tmp.e);
			dd->decRef(tdd.e);
			tdd = tmp;
			dd->garbageCollect();
		}
		else {
			dd::TDD temp_tdd = dd->cont(temp_tdd1, temp_tdd2);
			tdd = dd->cont(tdd, temp_tdd);
		}


		if (get_max_node) {
			node_num_max = dd->size(tdd.e);
			if (node_num_max > nodes[0]) {
				nodes[0] = node_num_max;
			}
		}
	}

	int node_num_final = dd->size(tdd.e);

	nodes[1] = node_num_final;
	std::cout << tdd.e.w << std::endl;

	if (release) {
		dd->decRef(tdd.e);
	}
	dd->garbageCollect();

	std::cout << "Done!!!" << std::endl;
	return nodes;
}


int* Simulate_with_partition2(std::string path, std::string  file_name, std::unique_ptr<dd::Package<>>& dd) {

	std::map<int, gate> gate_set = import_circuit(path + file_name);
	//std::map<int, std::vector<dd::Index>> Index_set = get_index(gate_set);
	int cx_cut_max = qubits_num / 2 + 1;
	std::map<int, map<int, std::vector<int>>>  par = cir_partition2(gate_set, cx_cut_max, qubits_num / 2);

	int* nodes = new int[2];
	nodes[0] = 0;
	nodes[1] = 0;

	std::map<std::string, int> var;

	var = get_var_order();

	std::map<int, std::vector<dd::Index>> Index_set = get_index(gate_set, var);

	dd->varOrder = var;

	dd::TDD tdd = { dd::Edge<dd::mNode>::one ,{} };

	if (release) {
		dd->incRef(tdd.e);
	}

	start = clock();
	int node_num_max = 0;

	//std::cout << par.size() << std::endl;

	for (int k = 0; k < par.size(); k++) {
		//std::cout << k << std::endl;
		dd::TDD temp_tdd1 = { dd::Edge<dd::mNode>::one ,{} };
		if (release) {
			dd->incRef(temp_tdd1.e);
		}
		for (int k1 = 0; k1 < par[k][0].size(); k1++) {
			int gate_idx = par[k][0][k1];
			string nam = gate_set[gate_idx].name;
			temp_tdd1 = apply(temp_tdd1, nam, Index_set[gate_idx], dd);
		}

		dd::TDD temp_tdd2 = { dd::Edge<dd::mNode>::one ,{} };
		if (release) {
			dd->incRef(temp_tdd2.e);
		}
		for (int k1 = 0; k1 < par[k][1].size(); k1++) {
			int gate_idx = par[k][1][k1];
			string nam = gate_set[gate_idx].name;
			temp_tdd2 = apply(temp_tdd2, nam, Index_set[gate_idx], dd);
		}

		dd::TDD temp_tdd3 = { dd::Edge<dd::mNode>::one ,{} };
		if (release) {
			dd->incRef(temp_tdd3.e);
		}
		for (int k1 = 0; k1 < par[k][2].size(); k1++) {
			int gate_idx = par[k][2][k1];
			string nam = gate_set[gate_idx].name;
			temp_tdd3 = apply(temp_tdd3, nam, Index_set[gate_idx], dd);
		}

		if (release) {
			dd::TDD temp_tdd = dd->cont(temp_tdd1, temp_tdd2);
			dd->decRef(temp_tdd1.e);
			dd->decRef(temp_tdd2.e);
			temp_tdd = dd->cont(temp_tdd, temp_tdd3);
			dd->decRef(temp_tdd3.e);
			auto tmp = dd->cont(tdd, temp_tdd);
			dd->incRef(tmp.e);
			dd->decRef(tdd.e);
			tdd = tmp;
			dd->garbageCollect();
		}
		else {
			dd::TDD temp_tdd = dd->cont(temp_tdd1, temp_tdd2);
			temp_tdd = dd->cont(temp_tdd, temp_tdd3);
			tdd = dd->cont(tdd, temp_tdd);
		}


		if (get_max_node) {
			node_num_max = dd->size(tdd.e);
			if (node_num_max > nodes[0]) {
				nodes[0] = node_num_max;
			}
		}
	}

	int node_num_final = dd->size(tdd.e);

	nodes[1] = node_num_final;
	std::cout << tdd.e.w << std::endl;

	//dd::DDdotExportMatrix(tdd.e, "ttt2");

	if (release) {
		dd->decRef(tdd.e);
	}
	dd->garbageCollect();

	std::cout << "Done!!!" << std::endl;
	return nodes;
}

//计算qubit_num
int get_qubits_num(std::string  file_name) {

	int qubits_num = 0;

	std::ifstream  infile;

	infile.open(file_name);

	std::string line;
	std::getline(infile, line);
	std::getline(infile, line);
	std::getline(infile, line);
	//std::getline(infile, line);
	while (std::getline(infile, line))
	{

		vector<string> g = split(line, " ");

		smatch result;

		if (g[0] == "cx") {

			regex pattern("q\\[(\\d+)\\],q\\[(\\d+)\\];");
			if (regex_match(g[1], result, pattern))
			{
				if (stoi(result[1]) > qubits_num) {
					qubits_num = stoi(result[1]);
				}
				if (stoi(result[2]) > qubits_num) {
					qubits_num = stoi(result[2]);
				}
			}

		}
		else {
			regex pattern("q\\[(\\d+)\\];");
			if (regex_match(g[1], result, pattern))
			{
				if (stoi(result[1]) > qubits_num) {
					qubits_num = stoi(result[1]);
				}
			}
		}
	}
	infile.close();
	qubits_num += 1;
	return qubits_num;
}

int get_qubits_num_from_circuit(std::string circuit) {

	std::stringstream ss(circuit);
	std::string line;
	while (std::getline(ss, line)) {
		if (line.find("qreg") != std::string::npos) {
			printf("I got to the first if with %s\n", line.c_str());
			smatch result;
			string qubit_str = split(line, " ")[1];
			regex pattern("qreg q\\[(\\d+)\\];");
			if (regex_match(qubit_str, result, pattern))
			{
				printf("I got to the second if with %s\n", result.str(1));
				return stoi(result[1]);
			}	
		}
	}

    // for (std::string line; std::getline(ss, line, '\n');) {
    //     if (line.find("qreg") != std::string::npos) {
	// 		printf("I got to the first if with %s\n", line);
	// 		smatch result;
	// 		string qubit_str = split(line, " ")[1];
	// 		regex pattern("qreg q\\[(\\d+)\\];");
	// 		if (regex_match(qubit_str, result, pattern))
	// 		{
	// 			printf("I got to the second if with %s and %s\n", result[0], result[1]);
	// 			return stoi(result[1]);
	// 		}	
	// 	}
	// }

	return -1;
}

//计算gate_num
int get_gates_num(std::string  file_name) {

	int gates_num = 0;

	std::ifstream  infile;

	infile.open(file_name);

	std::string line;
	std::getline(infile, line);
	std::getline(infile, line);
	std::getline(infile, line);
	//std::getline(infile, line);
	while (std::getline(infile, line))
	{
		gates_num += 1;
	}
	infile.close();
	return gates_num;
}

int get_gates_num_from_circuit(std::string circuit) {
	int totalLines = 0;

	for (int i = 0; i < circuit.size(); i++) {
		if (circuit[i] == ';') totalLines++;
	}

	return totalLines - 3;

}

std::vector<std::tuple<int, int>> get_actual_plan_from_string(string plan) {
	std::vector<std::tuple<int, int>> res;

	std::stringstream plan_stream(plan);
	std::string entry;

	while(std::getline(plan_stream, entry, ';')) {
		string new_str = entry.substr(1, entry.size()-1);

		std::stringstream entry_stream(new_str);
		std::string int_str;
		std::vector<int> members;

		while (std::getline(entry_stream, int_str, ',')) {
			members.push_back(stoi(int_str));
		}

		res.push_back({members[0], members[1]});
		members.clear();
	}

	return res;
}

//计算一个电路的tdd
int* Simulate_with_tdd(std::string path, std::string  file_name, std::unique_ptr<dd::Package<>>& dd) {

	std::map<int, gate> gate_set = import_circuit(path + file_name);

	int* nodes = new int[2];
	nodes[0] = 0;
	nodes[1] = 0;
	std::cout << "Start!!!" << std::endl;

	dd->varOrder = get_var_order();

	std::map<int, std::vector<dd::Index>> Index_set = get_index(gate_set, dd->varOrder);


	//开始仿真，每一步都要处理指标，生成当前门的temp_tdd,并与现有的结果收缩得到完整tdd


	dd::TDD tdd = { dd::Edge<dd::mNode>::one ,{} };

	if (release) {
		dd->incRef(tdd.e);
	}

	int node_num_max = 0;
	start = clock();

	for (int k = 0; k < gate_set.size(); k++) {
		{
			tdd = apply(tdd, gate_set[k].name, Index_set[k], dd);
			if (get_max_node) {
				node_num_max = dd->size(tdd.e);
				if (node_num_max > nodes[0]) {
					nodes[0] = node_num_max;
				}
			}

		}
	}



	std::cout << tdd.e.w << std::endl;
	//std::cout << "TDD: ";
	//for (const auto& element : tdd.key_2_index) {
	//	std::cout << element << " ";
	//}
	//std::cout << std::endl;

	int node_num_final = dd->size(tdd.e);
	nodes[1] = node_num_final;
	//dd->statistics();
	//dd::export2Dot(tdd.e, "tdd1");


	if (release) {
		dd->decRef(tdd.e);
	}
	dd->garbageCollect();

	std::cout << "Done!!!" << std::endl;
	return nodes;
}


std::tuple<dd::TDD, long> plannedContractionOnCircuit(std::string circuit, std::vector<std::tuple<int, int>> plan, 
														std::unique_ptr<dd::Package<>>& dd, std::string res_filename, bool debugging, bool make_dataset, json& result_data) {
	// Load in circuit from file
	std::map<int, gate> gate_set = import_circuit_from_string(circuit);
	//printf("Successfully imported circuit\n");

	int* nodes = new int[2];
	nodes[0] = 0;
	nodes[1] = 0;


	// Prepare TDD env (var order ...)
	dd->varOrder = get_var_order();
	dd->to_test = debugging;

	std::map<int, std::vector<dd::Index>> Index_set = get_index(gate_set, dd->varOrder);
	dd::TDD tdd = { dd::Edge<dd::mNode>::one ,{} };

	if (release) {
		dd->incRef(tdd.e);
	}

	int node_num_max = 0;

	// Prepare Gate TDDs
	std::vector<dd::TDD> gateTDDs(gate_set.size());
	for (int i = 0; i < gateTDDs.size(); i++) {
		//printf("Gate is: %s\n", gate_set[i].name.c_str());
		gateTDDs[i] = gateToTDD(gate_set[i].name, Index_set[i], dd);
		if (debugging) {
			printf((std::string("Gate ") + std::string(gate_set[i].name) + std::string(":\n")).c_str());
			for (int j = 0; j < gateTDDs[i].index_set.size(); j++) {
				printf("    Gate %d-%d: key name = %s, idx = %d\n", i, j, gateTDDs[i].index_set[j].key.c_str(), gateTDDs[i].index_set[j].idx);
			}
			for (int j = 0; j < gateTDDs[i].gates.size(); j++) {
				printf("    Gate %d-%d: gate name = %s, params = ", i, j, gateTDDs[i].gates[j].name.c_str());
				for (int k = 0; k < gateTDDs[i].gates[j].params.size(); k++) {
					printf("%f, ", gateTDDs[i].gates[j].params[k]);
				}
				printf("\n");
			}
			printf("\n");
			std::string gate_name_fs = gate_set[i].name;
			std::replace(gate_name_fs.begin(), gate_name_fs.end(), '(', '_');
			std::replace(gate_name_fs.begin(), gate_name_fs.end(), ')', '_');

			dd::export2Dot(gateTDDs[i].e, "cpp_debugging/" + res_filename + "_g" + std::to_string(i) + "_" + gate_name_fs);
		}
	}

	struct timeval start, end;
    long mtime, seconds, useconds;  

	if (debugging)
		printf("Starting contraction\n\n");
	gettimeofday(&start, NULL);
	// Apply plan
	int current_step = 1;
	std::vector<json> jsons(plan.size());
	for (int k = 0; k < plan.size(); k++) {
		{
			// if (((double) k) / ((double) plan.size()) * 10 > current_step) {
			// 	printf("Done with: %d of %ld\n", k, plan.size());
			// 	current_step++;
			// }
			std::string folder_name = std::string("cpp_debugging/contraction_") + std::to_string(k) + "/";

			int leftIndex = std::get<0>(plan[k]);
			int rightIndex = std::get<1>(plan[k]);

			dd::TDD leftTDD = gateTDDs[leftIndex];
			dd::TDD rightTDD = gateTDDs[rightIndex];

			if (debugging) {
				std::filesystem::create_directory(folder_name);
				dd::export2Dot(leftTDD.e, folder_name + res_filename + "_v" + std::to_string(k));
				dd::export2Dot(rightTDD.e, folder_name + res_filename + "_h" + std::to_string(k));
			}

			dd::TDD resTDD;
			if (make_dataset) {
				json res_json;
				resTDD = applyTDDsWithJSON(leftTDD, rightTDD, dd, res_json);
				jsons[k] = res_json;
			} else {
				resTDD = applyTDDs(leftTDD, rightTDD, dd);
			}
			
			// std::tuple<dd::TDD, json> res = applyTDDs(leftTDD, rightTDD, dd);
			// dd::TDD resTDD = std::get<0>(res);
			// json resJson = std::get<1>(res);
			
			


			if (get_max_node) {
				node_num_max = dd->size(resTDD.e);
				if (node_num_max > nodes[0]) {
					nodes[0] = node_num_max;
				}
			}

			gateTDDs[rightIndex] = resTDD;
			if (debugging) {
				dd::export2Dot(resTDD.e, folder_name + res_filename + "_r" + std::to_string(k));
			}

		}
	}
	gettimeofday(&end, NULL);

	if (debugging)
		printf("Done with contraction\n\n");
	seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

	if (debugging)
    	printf("Elapsed time: %ld milliseconds\n", mtime);

	result_data["data"] = jsons;
	// // Pretty print json file
	// std::ofstream out_file(folder_name + res_filename + "_r" + std::to_string(k) + ".json");
	// out_file << std::setw(4) << res_json << std::endl;
	// out_file.close();

	return {gateTDDs[std::get<1>(plan[plan.size() - 1])], mtime};
}

dd::TDD plannedContractionOnCircuitFromFile(std::string path, std::vector<std::tuple<int, int>> plan, std::string file_name, std::unique_ptr<dd::Package<>>& dd) {
	// Load in circuit from file
	std::map<int, gate> gate_set = import_circuit(path + file_name);

	int* nodes = new int[2];
	nodes[0] = 0;
	nodes[1] = 0;


	// Prepare TDD env (var order ...)
	dd->varOrder = get_var_order();

	std::map<int, std::vector<dd::Index>> Index_set = get_index(gate_set, dd->varOrder);
	dd::TDD tdd = { dd::Edge<dd::mNode>::one ,{} };

	if (release) {
		dd->incRef(tdd.e);
	}

	int node_num_max = 0;

	// Prepare Gate TDDs
	std::vector<dd::TDD> gateTDDs(gate_set.size());
	for (int i = 0; i < gateTDDs.size(); i++) {
		gateTDDs[i] = gateToTDD(gate_set[i].name, Index_set[i], dd);
	}

	// Apply plan

	for (int k = 0; k < plan.size(); k++) {
		{
			int leftIndex = std::get<0>(plan[k]);
			int rightIndex = std::get<1>(plan[k]);

			dd::TDD leftTDD = gateTDDs[leftIndex];
			dd::TDD rightTDD = gateTDDs[rightIndex];
			
			dd::TDD resTDD = applyTDDs(leftTDD, rightTDD, dd);
			// std::tuple<dd::TDD, json> res = applyTDDs(leftTDD, rightTDD, dd);
			// dd::TDD resTDD = std::get<0>(res);

			if (get_max_node) {
				node_num_max = dd->size(resTDD.e);
				if (node_num_max > nodes[0]) {
					nodes[0] = node_num_max;
				}
			}

			gateTDDs[rightIndex] = resTDD;

		}
	}

	return gateTDDs[std::get<1>(plan[plan.size() - 1])];
}



std::vector<std::tuple<int, int>> getDefaultPlan(int gates) {
	std::vector<std::tuple<int, int>> plan(gates - 1);

	for (int i = 0; i < gates - 1; i++) {
		plan[i] = { i, i + 1 };
	}

	return plan;
}
