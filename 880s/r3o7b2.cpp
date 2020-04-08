#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fstream>
#include <memory>
using namespace std;


using uint = unsigned int;

const int N = 6e5 + 10;
int realN;

typedef vector<vector<int>> VVI;

class Tarjan_vec {
    private:
        vector<int> dfn, low, s;
        vector<bool> vis;
        int TIME, tt;
        vector<unordered_set<int>>* sccs;
        VVI* g;

        void init() {
            for (int i = 0; i < realN; i++) dfn[i] = low[i] = vis[i] = 0;
            TIME = 1;
            tt = 0;
            sccs = nullptr;
            g = nullptr;
        }

        void dfs(int from) {
            dfn[from] = low[from] = TIME++;
            s[++tt] = from;
            vis[from] = true;
            for (auto to : (*g)[from]) {
                if (!dfn[to]) {
                    dfs(to);
                    low[from] = std::min(low[from], low[to]);
                }
                else if (vis[to]) {
                    low[from] = std::min(low[from], dfn[to]);
                }
            }

            // 找到了一个 scc
            if (dfn[from] == low[from]) {
                unordered_set<int> scc;
                int now;
                do {
                    now = s[tt--];
                    scc.insert(now);
                    vis[now] = false;
                } while (now != from);

                // 优化
                if (scc.size() >= 3) {
                    sccs->emplace_back(scc);
                }
            }
        }

    public:
        Tarjan_vec() {
            dfn = low = s = vector<int>(realN);
            vis = vector<bool>(realN);
        }

        void getSccs(decltype(g) g, decltype(sccs) sccs) {
            init();
            this->g = g;
            this->sccs = sccs;
            for (int i = 0; i < realN; i++)
                if (!dfn[i]) 
                    dfs(i);
        }
};

class Util {
public:
        VVI getSubGraphBySet1(const VVI& g, const unordered_set<int>& s)
        {
            VVI sub(realN);
            for (int from = 0; from < realN; from++)
            {
                if (s.count(from))
                    continue;

                for (auto to : g[from]) 
                {
                    if (!s.count(to)) 
                        sub[from].push_back(to);
                }
            }
            return sub;
            
        }
        VVI getSubGraphBySet2(const VVI& g, const unordered_set<int>& s, vector<int>& vertex_rank)
        {
            VVI sub(realN);
            vector<int> in(realN), out(realN);
            for (int from = 0; from < realN; from++)
                if (s.count(from)) 
                    for (auto to : g[from]) if (s.count(to)) {
                        sub[from].push_back(to);
                        in[to]++;
                        out[from]++;
                    }
            for (int i = 0; i < realN; i++) vertex_rank[i] = in[i] * out[i];
            return sub;
            
        }

        VVI uniformData(const VVI& vec) {
            VVI data;
            for (auto& item : vec) {
                auto it = min_element(item.begin(), item.end());
                vector<int> tmp(it, item.end());
                tmp.insert(tmp.end(), item.begin(), it);
                data.emplace_back(tmp);
            }

            sort(data.begin(), data.end(), [](const std::vector<int>& a, const std::vector<int>& b) {
                if (a.size() != b.size()) return a.size() < b.size();
                    return a < b;
            });

            data.erase(unique(data.begin(), data.end(), 
            [](const std::vector<int> & a, const std::vector<int>& b) {
                    if (a.size() != b.size()) return false;
                    for (int i = 0; i < a.size(); i++) if (a[i] != b[i]) return false;
                    return true;
            }), data.end());
            return data;
        }
};

class Johnson {
    private:
        vector<bool> blocked_set;
        vector<int> stk;
        int tt;
        vector<unordered_set<int>> blocked_map;
        VVI circles;

        void init() {
            tt = 0;
            for (int i = 0; i < realN; i++)
            {
                blocked_set[i] = false;
                blocked_map[i].clear();
            }
        }

        void unblock(int from) {
            blocked_set[from] = false;
            for (auto to : blocked_map[from])
                if (blocked_set[to]) unblock(to);
            blocked_map[from].clear();
        }

        bool dfs(const VVI& g, int start, int from) {
            stk[++tt] = from;
            blocked_set[from] = true;
            bool findCircle = false;

            for (auto to : g[from]) {
                if (to == start) {
                    if (tt >= 3 && tt <= 7) {
                        circles.emplace_back(stk.begin() + 1, stk.begin() + 1 + tt);
                    }
                    findCircle = true;
                    continue;
                }

                if (tt < 7 && !blocked_set[to]) {
                    findCircle |= dfs(g, start, to);
                }
            }

            if (tt >= 7) 
                findCircle = true;

            -- tt;
            if (findCircle) {
                unblock(from);
            } else {
                for (auto to : g[from]) 
                    blocked_map[to].insert(from);
            }
            return findCircle;
        }

    public:
        Johnson() {
            blocked_set.resize(realN);
            stk.resize(realN + 100);
            blocked_map.resize(realN);
            init();
        }

        VVI getAllCircles(VVI& g) {
            Tarjan_vec tarjan;
            Util util;

            vector<unordered_set<int>> sccs;
            tarjan.getSccs(&g, &sccs);

            while (!sccs.empty()) {

                unordered_set<int> usedVrtxes;

                for (const auto& scc : sccs) {
                    vector<int> rank(realN);
                    VVI subGraph = util.getSubGraphBySet2(g, scc, rank);
                    int start = *scc.begin();
                    for (int i = 0; i < realN; i++) if (rank[i] > rank[start]) start = i;

                    usedVrtxes.insert(start);
                    init();
                    dfs(subGraph, start, start);
                }

                vector<int> rank(realN);
                g = util.getSubGraphBySet1(g, usedVrtxes);

                sccs.clear();
                tarjan.getSccs(&g, &sccs);
            }
            return util.uniformData(circles);
        }
};

int main() {

    freopen("/data/test_data.txt", "r", stdin);
    freopen("/projects/student/result.txt", "w", stdout);

    ios::sync_with_stdio(false);
    cin.tie(0);

    unsigned int u, v, w;
    char c;
    vector<pair<uint, uint>> data;

    // 大范围 -> 小范围
    unordered_map<uint, uint> v_uhash;
    // 小范围 -> 大范围
    vector<uint> v_hash;

    while (std::cin >> u >> c >> v >> c >> w) {
        data.emplace_back(u, v);
        v_hash.push_back(u);
        v_hash.push_back(v);
    }

    std::sort(v_hash.begin(), v_hash.end());
    v_hash.erase(std::unique(v_hash.begin(), v_hash.end()), v_hash.end());

    for (int i = 0; i < v_hash.size(); i++) 
        v_uhash[v_hash[i]] = i;

    realN = v_uhash.size();
    VVI graph(realN);

    for (const auto& edge : data)
        graph[v_uhash[edge.first]].push_back(v_uhash[edge.second]);

    Johnson johnson;
    auto ans = johnson.getAllCircles(graph);

    std::cout << ans.size() << std::endl;
    for (const auto& vec : ans) {
        std::cout << v_hash[vec[0]];
        for (int i = 1; i < vec.size(); i++) 
            std::cout << "," << v_hash[vec[i]];
        std::cout << endl;
    }
    return 0;
}
