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
        for (auto to : g->at(from)) {
            if (!dfn[to]) {
                dfs(to);
                low[from] = min(low[from], low[to]);
            }
            else if (vis[to]) {
                low[from] = min(low[from], dfn[to]);
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
        init();
    }

    void getSccs(decltype(g) g, decltype(sccs) sccs) {
        this->g = g;
        this->sccs = sccs;
        for (int i = 0; i < realN; i++)
            if (!dfn[i]) dfs(i);
        init();
    }
};

class Util {
public:
    VVI getSubGraphBySet(const VVI& g, const unordered_set<int>& s, vector<int>& vertex_rank, bool exclude = false) {
        VVI sub(realN);
        vector<int> in(realN), out(realN);
        for (int from = 0; from < realN; from++)
            if (s.count(from) != exclude) for (auto to : g[from]) if (s.count(to) != exclude) {
                sub[from].push_back(to);
                in[to]++;
                out[from]++;
            }
        for (int i = 0; i < realN; i++) vertex_rank[i] = in[i] * out[i];
        return sub;
    }

    VVI uniformData(const VVI& vec) {
        VVI data;
        for (auto & item : vec) {
            auto it = min_element(item.begin(), item.end());
            vector<int> tmp(it, item.end());
            tmp.insert(tmp.end(), item.begin(), it);
            data.emplace_back(tmp);
        }

        sort(data.begin(), data.end(), [](auto a, auto b) {
            if (a.size() != b.size()) return a.size() < b.size();
            return a < b;
            });
        return data;
    }
};

class Johnson {
public:
    static int circle_count;        // 调试用，记录环的数量
    static int start_index;
    static int sccs_size;
    static int start_index_rank;

private:
    vector<bool> blocked_set;
    vector<int> stk;
    int tt;
    vector<unordered_set<int>> blocked_map;
    VVI circles;

    void init() {
        for (int i = 0; i < realN; i++) blocked_set[i] = false, blocked_map[i].clear();
        tt = 0;
    }

    void unblock(int from) {
        blocked_set[from] = false;
        for (auto to : blocked_map[from])
            if (blocked_set[to]) unblock(to);
        blocked_map[from].erase(from);
    }

    bool dfs(const VVI& g, int start, int from) {
        stk[++tt] = from;
        blocked_set[from] = true;
        bool findCircle = false;

        if (tt <= 7 && g[from].size()) {
            for (auto to : g[from]) {
                if (to == start) {
                    if (tt >= 3) {
                        circles.emplace_back(stk.begin() + 1, stk.begin() + 1 + tt);
                        circle_count++;
                    }
                    findCircle = true;
                }
                else if (tt < 7 && !blocked_set[to]) {
                    findCircle |= dfs(g, start, to);
                }
            }
        }

        tt--;
        if (findCircle) {
            unblock(from);
        }
        else {
            for (auto to : g[from]) blocked_map[to].insert(from);
        }
        return findCircle;
    }

public:
    Johnson() {
        blocked_set.resize(realN);
        stk.resize(realN);
        blocked_map.resize(realN);
        init();
    }

    VVI getAllCircles(VVI g) {
        Tarjan_vec tarjan;
        Util util;

        vector<unordered_set<int>> sccs;
        tarjan.getSccs(&g, &sccs);
        while (sccs.size()) {
            unordered_set<int> usedVrtxes;
            sccs_size = sccs.size();
            for (const auto& scc : sccs) {
                vector<int> rank(realN);
                VVI subGraph = util.getSubGraphBySet(g, scc, rank);
                int start = *scc.begin();
                for (int i = 0; i < realN; i++) if (rank[i] > rank[start]) start = i;;
                start_index = start;
                cout << start_index << " " << sccs_size << endl;
                start_index_rank = rank[start];
                usedVrtxes.insert(start);
                init();
                dfs(subGraph, start, start);
            }
            vector<int> rank(realN);
            g = util.getSubGraphBySet(g, usedVrtxes, rank, true);
            sccs.clear();
            tarjan.getSccs(&g, &sccs);
        }
        return util.uniformData(circles);
    }
};
int Johnson::circle_count = 0;
int Johnson::sccs_size = -1;
int Johnson::start_index = -1;
int Johnson::start_index_rank = -1;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    int u, v, w;
    char c;
    vector<pair<int, int>> data;
    vector<int> v_hash;
    unordered_map<int, int> v_uhash;

    while (cin >> u >> c >> v >> c >> w) {
        data.push_back({ u, v });
        v_hash.push_back(u);
        v_hash.push_back(v);
    }
    sort(v_hash.begin(), v_hash.end());
    v_hash.erase(unique(v_hash.begin(), v_hash.end()), v_hash.end());
    for (int i = 0; i < v_hash.size(); i++) v_uhash[v_hash[i]] = i;
    realN = v_uhash.size();

    VVI graph(realN);
    for (auto & edge : data) graph[v_uhash[edge.first]].push_back(v_uhash[edge.second]);


    Johnson johson;

    thread th([&johson, &v_hash]
        {
            int last_count = 0;
            while (true) {
                cout << "searched_circles: " << johson.circle_count << " speed: " << johson.circle_count - last_count << endl;
                // cout << "sccs's size: " << johson.sccs_size << " searching_index: " << v_hash[johson.start_index] << "start_index_rank: " << johson.start_index_rank << endl;
                last_count = johson.circle_count;
                this_thread::sleep_for(chrono::seconds(1));
            }
        });

    


    auto ans = johson.getAllCircles(graph);
    ios::sync_with_stdio(false);
    cin.tie(0);

    cout << ans.size() << endl;
    for (auto vec : ans) {
        cout << v_hash[vec[0]];
        for (int i = 1; i < vec.size(); i++) cout << "," << v_hash[vec[i]];
        cout << endl;
    }
}
