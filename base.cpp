#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>


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
    }

    void addEdge(int from, int to)
    {
        m_graph[from].push_back(to);
    }

    std::vector<std::vector<int>>& exec()
    {

        return m_cycles;
    }
private:  
    size_t m_vertex_number;
    std::vector<std::vector<int>> m_graph;
    std::vector<std::vector<int>> m_cycles;
};

int main(int argc, char* argv[])
{
    freopen("/data/test_data.txt", "r", stdin);
    freopen("/projects/student/result.txt", "w", stdout);

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
    for (size_t i = 0; i < vertex_number; ++ i)
        hash_map[vertexes[i]] = i;

    DirectedGraph directed_graph(vertex_number);
    for (const auto& item : data)
        directed_graph.addEdge(hash_map[item.first] + 1, 
                               hash_map[item.second] + 1);
    auto cycles = directed_graph.exec();
    std::cout << cycles.size() << std::endl;
    for (const auto& vec : cycles)
    {
        std::cout << vertexes[vec[0] - 1];   
        for (int i = 1; i < vec.size(); ++ i)
            std::cout << "," << vertexes[vec[i] - 1];
        std::cout << std::endl;
    }
    return 0;    
}
