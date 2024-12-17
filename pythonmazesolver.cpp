#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;

class Node {
public:
    pair<int, int> state;
    Node* parent;
    string action;

    Node(pair<int, int> state, Node* parent, string action)
        : state(state), parent(parent), action(action) {}
};

class StackFrontier {
public:
    vector<Node*> frontier;

    void add(Node* node) {
        frontier.push_back(node);
    }

    bool containsState(pair<int, int> state) {
        return any_of(frontier.begin(), frontier.end(), [state](const Node* node) {
            return node->state == state;
        });
    }

    bool empty() {
        return frontier.empty();
    }

    Node* remove() {
        if (empty()) {
            throw runtime_error("empty frontier");
        } else {
            Node* node = frontier.back();
            frontier.pop_back();
            return node;
        }
    }
};

class QueueFrontier : public StackFrontier {
public:
    Node* remove() {
        if (empty()) {
            throw runtime_error("empty frontier");
        } else {
            Node* node = frontier.front();
            frontier.erase(frontier.begin());
            return node;
        }
    }
};

class Maze {
public:
    int height;
    int width;
    pair<int, int> start;
    pair<int, int> goal;
    vector<vector<bool>> walls;
    set<pair<int, int>> explored;
    pair<vector<string>, vector<pair<int, int>>> solution;

    Maze(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Error opening file");
        }

        stringstream buffer;
        buffer << file.rdbuf();
        string contents = buffer.str();

        if (count(contents.begin(), contents.end(), 'A') != 1) {
            throw runtime_error("maze must have exactly one start point");
        }
        if (count(contents.begin(), contents.end(), 'B') != 1) {
            throw runtime_error("maze must have exactly one goal");
        }

        istringstream iss(contents);
        string line;
        while (getline(iss, line)) {
            vector<bool> row;
            for (char c : line) {
                if (c == 'A') {
                    start = { height, width };
                    row.push_back(false);
                } else if (c == 'B') {
                    goal = { height, width };
                    row.push_back(false);
                } else if (c == ' ') {
                    row.push_back(false);
                } else {
                    row.push_back(true);
                }
                width++;
            }
            walls.push_back(row);
            height++;
            width = 0;
        }

        solution = make_pair(vector<string>(), vector<pair<int, int>>());
    }

    void print() {
        auto solutionPath = (solution.second.empty()) ? vector<pair<int, int>>() : solution.second;

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (walls[i][j]) {
                    cout << "█";
                } else if (make_pair(i, j) == start) {
                    cout << "A";
                } else if (make_pair(i, j) == goal) {
                    cout << "B";
                } else if (find(solutionPath.begin(), solutionPath.end(), make_pair(i, j)) != solutionPath.end()) {
                    cout << "*";
                } else {
                    cout << " ";
                }
            }
            cout << endl;
        }
        cout << endl;
    }

    vector<pair<string, pair<int, int>>> neighbors(pair<int, int> state) {
        int row = state.first;
        int col = state.second;
        vector<pair<string, pair<int, int>>> candidates = {
            {"up", {row - 1, col}},
            {"down", {row + 1, col}},
            {"left", {row, col - 1}},
            {"right", {row, col + 1}}
        };

        vector<pair<string, pair<int, int>>> result;
        for (auto& candidate : candidates) {
            auto action = candidate.first;
            auto [r, c] = candidate.second;
            if (r >= 0 && r < height && c >= 0 && c < width && !walls[r][c]) {
                result.push_back({action, {r, c}});
            }
        }
        return result;
    }

    void solve() {
        int numExplored = 0;
        Node* startNode = new Node(start, nullptr, "");
        QueueFrontier frontier;
        frontier.add(startNode);

        while (true) {
            if (frontier.empty()) {
                throw runtime_error("no solution");
            }

            Node* node = frontier.remove();
            numExplored++;

            if (node->state == goal) {
                vector<string> actions;
                vector<pair<int, int>> cells;

                while (node->parent != nullptr) {
                    actions.push_back(node->action);
                    cells.push_back(node->state);
                    node = node->parent;
                }

                reverse(actions.begin(), actions.end());
                reverse(cells.begin(), cells.end());

                solution = make_pair(actions, cells);
                return;
            }

            explored.insert(node->state);

            for (auto [action, state] : neighbors(node->state)) {
                if (!frontier.containsState(state) && explored.find(state) == explored.end()) {
                    Node* child = new Node(state, node, action);
                    frontier.add(child);
                }
            }
        }
    }

    void outputImage(const string& filename, bool showSolution = true, bool showExplored = false) {
        const int cellSize = 50;
        const int cellBorder = 2;

        vector<pair<int, int>> solutionPath = (solution.second.empty()) ? vector<pair<int, int>>() : solution.second;

        vector<vector<int>> img(height, vector<int>(width, 0));

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (walls[i][j]) {
                    img[i][j] = 1;
                } else if (make_pair(i, j) == start) {
                    img[i][j] = 2;
                } else if (make_pair(i, j) == goal) {
                    img[i][j] = 3;
                } else if (find(solutionPath.begin(), solutionPath.end(), make_pair(i, j)) != solutionPath.end()) {
                    img[i][j] = 4;
                } else if (showExplored && explored.find(make_pair(i, j)) != explored.end()) {
                    img[i][j] = 5;
                } else {
                    img[i][j] = 0;
                }
            }
        }

        ofstream imageFile(filename, ios::binary);

        imageFile << "P6\n" << width * cellSize << " " << height * cellSize << "\n255\n";

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int fill;
                if (img[i][j] == 1) {
                    fill = 40;
                } else if (img[i][j] == 2) {
                    fill = 255;
                } else if (img[i][j] == 3) {
                    fill = 0;
                } else if (img[i][j] == 4) {
                    fill = 220;
                } else if (img[i][j] == 5) {
                    fill = 212;
                } else {
                    fill = 237;
                }

                for (int k = 0; k < cellSize; k++) {
                    for (int l = 0; l < cellSize; l++) {
                        imageFile << static_cast<char>(fill);
                        imageFile << static_cast<char>(fill);
                        imageFile << static_cast<char>(fill);
                        imageFile << static_cast<char>(255);  // Alpha channel
                    }
                }
            }
        }

        imageFile.close();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " maze.txt" << endl;
        return 1;
    }

    try {
        Maze m(argv[1]);
        cout << "Maze:" << endl;
        m.print();
        cout << "Solving..." << endl;
        m.solve();
        cout << "States Explored: " << m.explored.size() << endl;
        cout << "Solution:" << endl;
        m.print();
        m.outputImage("maze.png", true, true);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
