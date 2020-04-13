#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <list>
#include <map>


int circle_cnt = 0;

// 注：图本身不进行离散化处理
class DirectedGraph
{
public:
    // 参数 vertex_number 表示有向图的顶点数量
    // 顶点的取值范围是 [1, vertex_number]
    explicit DirectedGraph(size_t vertex_number)
        : m_vertex_number(vertex_number)
    {
        m_graph.resize(m_vertex_number + 1);
        m_graph_neg.resize(m_vertex_number + 1);
        map1.resize(m_vertex_number + 1);
    }

    void addEdge(int from, int to)
    {
        m_graph[from].push_back(to);
        m_graph_neg[to].push_back(from);
    }

    std::vector<std::vector<int>>& exec()
    {
        

        auto start = clock();

        for (int i = 1; i <= m_vertex_number; i++) {
            solve(i);

            
        }

        auto end = clock();
        std::cout << (1.0 * end - start) / CLOCKS_PER_SEC << std::endl;

        for (auto& cycle : m_cycles) {
            auto it = std::min_element(cycle.begin(), cycle.end());
            std::vector<int> tmp(it, cycle.end());
            tmp.insert(tmp.end(), cycle.begin(), it);
            cycle = std::move(tmp);
        }
        std::sort(m_cycles.begin(), m_cycles.end(), 
            [](const std::vector<int>& lhs, const std::vector<int>& rhs)
            {
                if (lhs.size() != rhs.size())
                    return lhs.size() < rhs.size();
                return lhs < rhs;
            });

        m_cycles.erase(std::unique(m_cycles.begin(), m_cycles.end()), m_cycles.end());

        return m_cycles;
    }

    void solve(int i) {
        for (auto from1 : m_graph[i])
            if (from1 > i)
                for (auto from2 : m_graph[from1])
                    if (from2 > i && from2 != from1)
                        for (auto from3 : m_graph[from2])
                            if (from3 >= i && from3 != from2 && from3 != from1) {
                                if (from3 == i) {
                                    m_cycles.emplace_back(std::vector<int>{ i, from1, from2 });
                                    continue;
                                }
                                for (auto from4 : m_graph[from3])
                                    if (from4 >= i && from4 != from3 && from4 != from2 && from4 != from1)
                                        if (from4 == i) {
                                            m_cycles.emplace_back(std::vector<int>{ i, from1, from2, from3 });
                                        }
                                        else {
                                            map1[i][from4].emplace_back(std::vector<int>{ from1, from2, from3 });
                                        }
                            }



        for (auto from1 : m_graph_neg[i])
            if (from1 > i) {
                if (map1[i].count(from1)) {
                    for (const auto& sub_path : map1[i][from1]) {
                        std::vector<int> path = { i, sub_path[0], sub_path[1], sub_path[2], from1 };
                        int cnt = 0;
                        for (int i = 0; i < path.size(); i++)
                            for (int j = i + 1; j < path.size(); j++)
                                if (path[i] == path[j]) cnt++;
                        if (!cnt) m_cycles.push_back(path), circle_cnt++;
                    }
                }
                for (auto from2 : m_graph_neg[from1])
                    if (from2 > i && from2 != from1) {
                        if (map1[i].count(from2))
                            for (const auto& sub_path : map1[i][from2]) {
                                std::vector<int> path = { i, sub_path[0], sub_path[1], sub_path[2], from2, from1 };
                                int cnt = 0;
                                for (int i = 0; i < path.size(); i++)
                                    for (int j = i + 1; j < path.size(); j++)
                                        if (path[i] == path[j]) cnt++;
                                if (!cnt) m_cycles.push_back(path), circle_cnt++;
                            }
                        for (auto from3 : m_graph_neg[from2])
                            if (from3 > i && from3 != from2 && from3 != from1 && map1[i].count(from3))
                                for (const auto& sub_path : map1[i][from3]) {
                                    std::vector<int> path = { i, sub_path[0], sub_path[1], sub_path[2], from3, from2, from1 };
                                    int cnt = 0;
                                    for (int i = 0; i < path.size(); i++)
                                        for (int j = i + 1; j < path.size(); j++)
                                            if (path[i] == path[j]) cnt++;
                                    if (!cnt) m_cycles.push_back(path), circle_cnt++;
                                }
                    }
            }

        map1[i].erase(map1[i].begin(), map1[i].end());
    }

private:
    size_t m_vertex_number;
    std::vector<std::vector<int>> m_graph;
    std::vector<std::vector<int>> m_graph_neg;
    std::vector<std::vector<int>> m_cycles;

    std::vector < std::unordered_map<int, std::vector<std::vector<int>>>> map1;
};

int main(int argc, char* argv[])
{
    freopen("./test_data.txt", "r", stdin);
    freopen("./test_data.txt", "w", stdout);

    std::ios::sync_with_stdio(false);
    std::cin.tie(0);

    std::unordered_map<unsigned int, unsigned int> hash_map;
    std::vector<unsigned int> vertexes;

    unsigned int from, to, money;
    char delim;
    std::vector<std::pair<unsigned int, unsigned int>> data;
    while (std::cin >> from >> delim >> to >> delim >> money)
    {
        data.emplace_back(from, to);
        vertexes.push_back(from);
        vertexes.push_back(to);
    }

    std::sort(vertexes.begin(), vertexes.end());
    vertexes.erase(std::unique(vertexes.begin(), vertexes.end()),
        vertexes.end());
    size_t vertex_number = vertexes.size();
    for (size_t i = 0; i < vertex_number; ++i)
        hash_map[vertexes[i]] = i;

    DirectedGraph directed_graph(vertex_number);
    for (const auto& item : data)
        directed_graph.addEdge(hash_map[item.first] + 1,
            hash_map[item.second] + 1);

    std::thread debug([] {
        int last = 0;
        while (true) {
            std::cout << circle_cnt << " " << circle_cnt - last << std::endl;
            last = circle_cnt;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        });

    auto cycles = directed_graph.exec();

    std::cout << cycles.size() << std::endl;
    for (const auto& vec : cycles)
    {
        std::cout << vertexes[vec[0] - 1];
        for (int i = 1; i < vec.size(); ++i)
            std::cout << "," << vertexes[vec[i] - 1];
        std::cout << std::endl;
    }
    if (debug.joinable()) debug.join();
    return 0;
}
