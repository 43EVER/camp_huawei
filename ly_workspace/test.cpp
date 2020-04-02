#include <iostream>
#include <stack>
#include <vector>
#include <unordered_map>
#include <string.h>
#include <algorithm>
using namespace std;

const int MAXN = 1e6 + 10;

int COLOR;                          // 染色


vector<int> G[2][MAXN];     // 正向图0，反向图1
int dep[MAXN];                      // 记录节点深度，方便推测环的长度
int path[MAXN];                     // 记录节点路径，方便反推环的路径
bool vis[MAXN];
int color[MAXN];                    // 每个节点对应的颜色

stack<int> s;                       // 某个点的终止时间越靠后的，越接近栈顶 

vector<vector<int>> ans;

void positive_dfs(int pos) {
    vis[pos] = true;
    for (auto to : G[0][pos]) if (!vis[to]) positive_dfs(to);
    s.push(pos);
}

void negative_dfs(int pos) {
    color[pos] = COLOR;
    vis[pos] = false;
    for (auto to : G[1][pos]) if (vis[to]) negative_dfs(to);
}

void dfs(int pos) {
    vis[pos] = true;
    for (auto to : G[0][pos]) {
        if (color[to] != color[pos]) continue;  // 只遍历当前强连通分量的节点
        
        path[pos] = to;
        if (dep[to]) {
            int len = dep[pos] - dep[to] + 1;
            if (len < 3 || len > 7) continue;

            vector<int> tmp;
            for (int i = to; i != pos; i = path[i]) tmp.push_back(i);
            tmp.push_back(pos);
            auto min_it = min_element(tmp.begin(), tmp.end());
            vector<int> tmp2(min_it, tmp.end());
            for (auto it = tmp.begin(); it != min_it; it++) tmp2.push_back(*it);
            ans.push_back(tmp2);
        } else {
            dep[to] = dep[pos] + 1;
            dfs(to);
            dep[to] = 0;
        }
    }
    vis[pos] = false;
}

bool cmp(const vector<int> & a, const vector<int> & b) {
    if (a.size() != b.size()) return a.size() < b.size();
    for (int i = 0; i < a.size(); i++)
        if (a[i] != b[i]) return a[i] < b[i];
    return true;
}

bool equal1(const vector<int> & a, const vector<int> & b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++)
        if (a[i] != b[i]) return false;
    return true;
}

int main() {
    int N = 0;
    int u, v, w;
    char ch;
    while (cin >> u >> ch >> v >> ch >> w) {
        G[0][u].push_back(v);
        G[1][v].push_back(u);
        N = max(N, max(u, v));
    }

    for (int i = 0; i <= N; i++) if (!vis[i]) positive_dfs(i);
    while (s.size()) {
        int pos = s.top(); s.pop();
        if (vis[pos]) {
            COLOR++;
            negative_dfs(pos);
        }
    }

    memset(vis, false, sizeof(vis));
    unordered_map<int, bool> h;     // 颜色用过没用过
    for (int i = 0; i <= N; i++)
        if (!h.count(color[i])) {
            dep[i] = 1;
            dfs(i);
            h[color[i]] = true;
        }
    sort(ans.begin(), ans.end(), cmp);
    ans.erase(unique(ans.begin(), ans.end(), equal1), ans.end());
    cout << ans.size() << endl;
    for (auto item : ans) {
        for (auto i : item) cout << i << " ";
        cout << endl;
    }
}
