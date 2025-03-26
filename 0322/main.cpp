#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <algorithm>

using namespace std;

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
            clause.push_back(stoi(cell));
        }
        if (clause.size() == 3) {
            clauses.push_back(clause);
        }
    }
    infile.close();
    return clauses;
}

// 根據所有子句推算變數數量 D（假設變數編號 1~D）
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
    string filename;
    if (argc < 2) {
        // 直接指定完整路徑 (Windows 範例)
        filename = "C:\\Users\\benso\\Desktop\\AI\\[1132]AI_Assignment1_data\\3SAT_Dim=50.csv";
    } else {
        filename = argv[1];
    }

    vector<vector<int>> clauses = readClauses(filename);
    int D = getVariableCount(clauses);

    // 使用 BFS 進行搜尋，利用 queue 存放部分解（每個部分解為一個 int 向量）
    queue<vector<int>> q;
    q.push(vector<int>()); // 起始為空指派
    bool solutionFound = false;
    vector<int> solution;
    int expandedNodes = 0;

    while (!q.empty()) {
        vector<int> current = q.front();
        q.pop();
        expandedNodes++;

        // 若此部分解無法滿足子句（或無法可能滿足），則捨棄
        if (!isPromising(clauses, current)) {
            continue;
        }

        // 若已達完整指派，檢查是否為解
        if (current.size() == (size_t)D) {
            if (isSolution(clauses, current, D)) {
                solutionFound = true;
                solution = current;
                break;
            }
            continue;
        }

        // 擴展下一個變數，分別嘗試 false (0) 與 true (1)
        vector<int> childFalse = current;
        vector<int> childTrue = current;
        childFalse.push_back(0);
        childTrue.push_back(1);
        q.push(childFalse);
        q.push(childTrue);
    }

    // 將結果寫入 result.txt 並同時印在終端機
    ofstream outfile("result.txt");
    if (!outfile.is_open()) {
        cerr << "Error writing to result.txt" << endl;
        return 1;
    }
    if (solutionFound) {
        outfile << "Solution found. Expanded nodes: " << expandedNodes << "\n";
        outfile << "Assignment:";
        cout << "Solution found. Expanded nodes: " << expandedNodes << "\n";
        cout << "Assignment:";
        for (int val : solution) {
            outfile << " " << val;
            cout << " " << val;
        }
        outfile << "\n";
        cout << "\n";
    } else {
        outfile << "No solution found.\n";
        cout << "No solution found. Expanded nodes: " << expandedNodes << "\n";
    }
    outfile.close();
    return 0;
}
