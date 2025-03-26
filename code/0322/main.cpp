#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <string>

using namespace std;
namespace fs = std::filesystem;

// 檢查目前部分解是否仍有可能滿足所有子句
bool isPromising(const vector<vector<int>> &clauses, const vector<int> &assignment) {
    for (const auto &clause : clauses) {
        bool clauseSatisfied = false;
        bool allAssigned = true;
        // 檢查子句中的每個文字
        for (int lit : clause) {
            int varIndex = abs(lit) - 1;
            if (varIndex < assignment.size()) { // 該變數已被指派
                int val = assignment[varIndex];
                // 正文字需為 1，否定文字需為 0
                if ((lit > 0 && val == 1) || (lit < 0 && val == 0)) {
                    clauseSatisfied = true;
                    break;
                }
            } else {
                // 尚未指派的變數代表該子句仍有機會滿足
                allAssigned = false;
            }
        }
        // 若所有變數皆已指派且該子句不滿足，則此部分解不可行
        if (!clauseSatisfied && allAssigned) {
            return false;
        }
    }
    return true;
}

// 檢查完整指派是否滿足所有子句
bool isSolution(const vector<vector<int>> &clauses, const vector<int> &assignment, int D) {
    if (assignment.size() != D) return false;
    for (const auto &clause : clauses) {
        bool satisfied = false;
        for (int lit : clause) {
            int varIndex = abs(lit) - 1;
            int val = assignment[varIndex];
            if ((lit > 0 && val == 1) || (lit < 0 && val == 0)) {
                satisfied = true;
                break;
            }
        }
        if (!satisfied)
            return false;
    }
    return true;
}

// 讀取 CSV 檔案，假設每行有 3 個以逗號分隔的整數，代表一個子句
vector<vector<int>> readClauses(const string &filename) {
    ifstream infile(filename);
    vector<vector<int>> clauses;
    if (!infile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }
    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string cell;
        vector<int> clause;
        while (getline(ss, cell, ',')) {
            // 去除前後空白
            cell.erase(0, cell.find_first_not_of(" \t"));
            cell.erase(cell.find_last_not_of(" \t") + 1);
            if (!cell.empty()) {
                // 移除前置的 '+' 符號
                if (cell[0] == '+') {
                    cell = cell.substr(1);
                }
                clause.push_back(stoi(cell));
            }
        }
        if (clause.size() == 3) {
            clauses.push_back(clause);
        }
    }
    infile.close();
    return clauses;
}

// 根據所有子句推算變數數量 D（假設變數編號從 1 到 D）
int getVariableCount(const vector<vector<int>> &clauses) {
    int D = 0;
    for (const auto &clause : clauses) {
        for (int lit : clause) {
            D = max(D, abs(lit));
        }
    }
    return D;
}

int main(int argc, char* argv[]) {

    string folderPath;
    if (argc < 2) {
        folderPath = "../data";  // 與執行檔同目錄下的子資料夾
    } else {
        folderPath = argv[1];
    }

    // 迭代指定資料夾中所有副檔名為 .csv 的檔案
    for (const auto &entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".csv") {
            string filename = entry.path().string();
            cout << "Processing file: " << filename << endl;
            vector<vector<int>> clauses = readClauses(filename);
            int D = getVariableCount(clauses);

            // 使用 BFS 搜尋，queue 存放部分指派 (vector<int>)
            queue<vector<int>> q;
            q.push(vector<int>()); // 起始為空指派
            bool solutionFound = false;
            vector<int> solution;
            int expandedNodes = 0;

            while (!q.empty()) {
                vector<int> current = q.front();
                q.pop();
                expandedNodes++;

                // 若此部分解無法滿足子句，則捨棄
                if (!isPromising(clauses, current)) {
                    continue;
                }

                // 若已達完整指派，檢查是否滿足所有子句
                if (current.size() == (size_t)D) {
                    if (isSolution(clauses, current, D)) {
                        solutionFound = true;
                        solution = current;
                        break;
                    }
                    continue;
                }

                // 擴展下一個變數，嘗試 false (0) 與 true (1)
                vector<int> childFalse = current;
                vector<int> childTrue = current;
                childFalse.push_back(0);
                childTrue.push_back(1);
                q.push(childFalse);
                q.push(childTrue);
            }

            // 產生結果檔案名稱，例如 "3SAT_Dim=10_result.txt"
            string resultFilename = entry.path().stem().string() + "_result.txt";
            // 結果檔案存放在同一個資料夾中
            ofstream outfile(resultFilename);
            if (!outfile.is_open()) {
                cerr << "Error writing to file: " << resultFilename << endl;
                continue;
            }
            if (solutionFound) {
                outfile << "Solution found. Expanded nodes: " << expandedNodes << "\n";
                outfile << "Assignment:";
                cout << "File: " << filename << "\n";
                cout << "Solution found. Expanded nodes: " << expandedNodes << "\n";
                cout << "Assignment:";
                for (int val : solution) {
                    outfile << " " << val;
                    cout << " " << val;
                }
                outfile << "\n\n";
                cout << "\n";
            } else {
                outfile << "No solution found.\n";
                cout << "File: " << filename << "\nNo solution found. Expanded nodes: " << expandedNodes << "\n\n";
            }
            outfile.close();
        }
    }
    return 0;
}
