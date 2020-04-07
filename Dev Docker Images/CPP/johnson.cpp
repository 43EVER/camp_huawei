#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fstream>
using namespace std;

typedef unordered_map<int, unordered_set<int>> Graph;

class Tarjan {
private:
    unordered_set<int> vis;
    stack<int> s;
    unordered_map<int, int> dfn, low;
    int TIME = 1;
    const Graph& g;
    vector<unordered_set<int>> sccs;

    void dfs(int from) {
        s.push(from); vis.insert(from);
        dfn.insert({ from, TIME });
        low.insert({ from, TIME });
        TIME++;
        if (g.count(from)) {
            const auto& edges = g.find(from)->second;
            for (auto to : edges) {
                if (!dfn.count(to)) {
                    dfs(to);
                    low[from] = min(low[from], low[to]);
                } else if (vis.count(to)) {
                    low[from] = min(low[from], dfn[to]);
                }
            }
        }
        // 找到了点
        if (dfn[from] == low[from]) {
            unordered_set<int> tmp;
            int now;
            do {
                now = s.top(); s.pop();
                tmp.insert(now);
                vis.erase(now);
            } while (now != from);

            // 优化，小于 3 的强连通分量直接忽略
            if (tmp.size() >= 3) sccs.push_back(tmp);
        }
    }

public:
    Tarjan(decltype(g) g) : g(g) {}
    vector<unordered_set<int>> getSCCs() {
        for (const auto& p : g) {
            if (!dfn.count(p.first)) dfs(p.first);
        }
        return sccs;
    }
};


class Util {
public:
    Graph getSubGraphByVertex(const Graph& g, const unordered_set<int>& vertexes, bool exclude = false) {
        Graph subGraph;
        for (const auto& vertex : g) {
            int from = vertex.first;
            if (vertexes.count(from) == exclude) continue;
            for (auto to : vertex.second)
                if (!exclude && vertexes.count(from) && vertexes.count(to)) subGraph[from].insert(to);
                else if (exclude && !vertexes.count(from) && !vertexes.count(to)) subGraph[from].insert(to);
        }
        return subGraph;
    }
};

class Johnson {
private:
    stack<int> stk;
    unordered_set<int> blockedSet;
    unordered_map<int, unordered_set<int>> blockedMap;
public:
    vector<vector<int>> circles;
    vector<vector<int>> getAllCircles(Graph g) {
        auto sccs = Tarjan(g).getSCCs();
        Util util;

        while (sccs.size()) {
            unordered_set<int> usedVertex;
            for (auto scc : sccs) {
                auto subGraph = util.getSubGraphByVertex(g, scc);
                int startVertex = subGraph.begin()->first;
                dfs(subGraph, startVertex, startVertex);
                usedVertex.insert(startVertex);
                stk = stack<int>();
                blockedSet.clear();
                blockedMap.clear();
            }

            g = util.getSubGraphByVertex(g, usedVertex, true);
            sccs = Tarjan(g).getSCCs();
        }
        return circles;
    }

    bool dfs(const Graph& g, int startindex, int curindex) {
        bool findCircle = false;
        stk.push(curindex);
        blockedSet.insert(curindex);

        if (stk.size() <= 7 && g.count(curindex)) {
            auto edges = g.find(curindex)->second;
            int from = curindex;
            for (auto to : edges) {
                if (to == startindex) {
                    auto cpy = stk;
                    if (stk.size() >= 3 && stk.size() <= 7) {
                        vector<int> cycle;
                        while (cpy.size()) {
                            cycle.push_back(cpy.top());
                            cpy.pop();
                        }
                        reverse(cycle.begin(), cycle.end());
                        circles.push_back(cycle);
                        //for (auto i : cycle) cout << i << " ";
                        //cout << endl;
                    }
                    findCircle = true;
                }
                else if (!blockedSet.count(to) && stk.size() < 7) {
                    findCircle |= dfs(g, startindex, to);
                }
            }
        }

        if (findCircle) {
            unblock(curindex);
        }
        else {
            if (g.count(curindex)) {
                auto edges = g.find(curindex)->second;
                int from = curindex;
                for (auto to : edges) {
                    blockedMap[to].insert(from);
                }
            }
        }
        stk.pop();
        return findCircle;
    }
    void unblock(int index) {
        blockedSet.erase(index);
        if (blockedMap.count(index)) {
            for (auto to : blockedMap[index])
                if (blockedSet.count(to)) unblock(to);
            blockedMap.erase(index);
        }
    }
};

int main() {
    fstream fs;
    fs.open("./a.in", ios::in);

    int u, v, w;
    char c;
    Graph g;
    while (fs >> u >> c >> v >> c >> w)
        g[u].insert(v);

    for (auto &v : g) {
        if (v.second.size() > 10) cout << v.first << " ";
        while (v.second.size() > 10) v.second.erase(v.second.begin());
    }

    Johnson johnson;
    std::thread th([&johnson]
        {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << johnson.circles.size() << std::endl;
            }
        });

    auto ans = johnson.getAllCircles(g);

     decltype(ans) ans2;
     for (auto & vec : ans) {
         auto it = min_element(vec.begin(), vec.end());
         vector<int> tmp(it, vec.end());
         tmp.insert(tmp.end(), vec.begin(), it);
         ans2.push_back(tmp);
     }
     sort(ans2.begin(), ans2.end(), [](const auto & a, const auto & b) {
         if (a.size() != b.size()) return a.size() < b.size();
         return a < b;
     });

     cout << ans2.size() << endl;
     for (auto vec : ans2) {
         for (auto i : vec) cout << i << ",";
         cout << endl;
     }
}
