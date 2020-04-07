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

typedef vector<vector<int>> VVI;

class Tarjan_vec {
private:
    vector<int> dfn, low, s;
    vector<bool> vis;
    int TIME, tt;
    vector<unordered_set<int>>* sccs;
    VVI* g;

    void init() {
        for (int i = 0; i < N; i++) dfn[i] = low[i] = vis[i] = 0;
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
        dfn = low = s = vector<int>(N);
        vis = vector<bool>(N);
        init();
    }

    void getSccs(decltype(g) g, decltype(sccs) sccs) {
        this->g = g;
        this->sccs = sccs;
        for (int i = 0; i < N; i++)
            if (!dfn[i]) dfs(i);
        init();
    }
};

class Util {
public:
    VVI getSubGraphBySet(const VVI& g, const unordered_set<int>& s, bool exclude = false) {
        VVI sub(N);
        for (int from = 0; from < N; from++)
            if (s.count(from) != exclude) for (auto to : g[from]) if (s.count(to) != exclude) sub[from].push_back(to);
        return sub;
    }

    void uniformData(VVI& data) {
        sort(data.begin(), data.end(), [](auto a, auto b) {
            if (a.size() != b.size()) return a.size() < b.size();
            return a < b;
            });
    }
};

class Johnson {
public:
    static int circle_count;        // 调试用，记录环的数量

private:
    vector<bool> blocked_set;
    vector<int> stk;
    int tt;
    vector<unordered_set<int>> blocked_map;
    VVI circles;

    void init() {
        for (int i = 0; i < N; i++) blocked_set[i] = false, blocked_map[i].clear();
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
                    if (tt >= 3) circles.emplace_back(vector<int>(stk.begin() + 1, stk.begin() + 1 + tt)), circle_count++;
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
        blocked_set = vector<bool>(N);
        stk = vector<int>(N);
        blocked_map = decltype(blocked_map)(N);
        init();
    }

    VVI getAllCircles(VVI g) {
        Tarjan_vec tarjan;
        Util util;

        vector<unordered_set<int>> sccs;
        tarjan.getSccs(&g, &sccs);
        while (sccs.size()) {
            unordered_set<int> usedVrtxes;
            for (const auto& scc : sccs) {
                int start = *scc.begin();
                usedVrtxes.insert(start);
                init();
                dfs(util.getSubGraphBySet(g, scc), start, start);
            }
            g = util.getSubGraphBySet(g, usedVrtxes, true);
            sccs.clear();
            tarjan.getSccs(&g, &sccs);
        }
        util.uniformData(circles);
        return circles;
    }
};
int Johnson::circle_count = 0;


int main() {
    int u, v, w;
    char c;
    vector<pair<int, int>> data;
    vector<int> v_hash;
    unordered_map<int, int> v_uhash;
    while (cin >> u >> c >> v >> c >> w)
        data.push_back({ u, v }), v_hash.push_back(u), v_hash.push_back(v);
    sort(v_hash.begin(), v_hash.end());
    v_hash.erase(unique(v_hash.begin(), v_hash.end()), v_hash.end());
    for (int i = 0; i < v_hash.size(); i++) v_uhash[v_hash[i]] = i;

    VVI graph(N);
    for (auto edge : data) graph[v_uhash[edge.first]].push_back(v_uhash[edge.second]);

    for (int i = 0; i < N; i++) while (graph[i].size() > 10) graph[i].pop_back();

    Johnson johson;

    thread th([&johson]
        {
            while (true) {
                this_thread::sleep_for(chrono::seconds(1));
                cout << johson.circle_count << endl;
            }
        });

    auto ans = johson.getAllCircles(graph);
    ios::sync_with_stdio(false);
    cin.tie(0);

    cout << ans.size() << endl;
}
