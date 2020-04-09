#include <algorithm>
#include <cassert>
#include <iostream>
#include <stack>
#include <unordered_map>
#include <vector>


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
        m_belong.resize(m_vertex_number + 1);
    }

    void addEdge(int from, int to)
    {
        m_graph[from].push_back(to);    
    }

    std::vector<std::vector<int>>& exec()
    {
        Tarjan tarjan(*this);        
        tarjan.run();

        SimpleCycle simple_cycle(*this);
        for (int i = 0; i < m_sccs.size(); ++ i)
            simple_cycle.run(i); 

        for (auto& cycle : m_cycles)
        {
            auto iter = std::min_element(cycle.begin(), cycle.end());
            std::vector<int> tmp(iter, cycle.end());
            tmp.insert(tmp.end(), cycle.begin(), iter);      
            cycle = std::move(tmp);
        }
        std::sort(m_cycles.begin(), m_cycles.end(),
        [](const std::vector<int>& lhs, const std::vector<int>& rhs)
        {
            if (lhs.size() != rhs.size())
                return lhs.size() < rhs.size();
            return lhs < rhs; 
        });
        m_cycles.erase(std::unique(m_cycles.begin(), m_cycles.end()),
                       m_cycles.end());

        return m_cycles;
    }
private: 
    size_t m_vertex_number;
    std::vector<std::vector<int>> m_graph;

    size_t m_scc_flag{ 0 };
    std::vector<int> m_belong;
    std::vector<std::vector<int>> m_sccs;

    std::vector<std::vector<int>> m_cycles;

    class Tarjan
    {
    public:
        explicit Tarjan(DirectedGraph& graph)
            : dg(graph)
        {
            m_time.resize(dg.m_vertex_number + 1);
            m_low.resize(dg.m_vertex_number + 1);
            m_exist.resize(dg.m_vertex_number + 1); 
        }

        void run()
        {
            for (int i = 1; i <= dg.m_vertex_number; ++ i)
                if (m_time[i] == 0)
                    search(i); 
        }
    private: 
        void search(int from)
        {
            ++ m_timestamp;
            m_time[from] = m_low[from] = m_timestamp;
            
            m_record.push(from);
            m_exist[from] = true;
            
            for (const auto& to : dg.m_graph[from])
            {
                if (m_time[to] == 0)
                {
                    search(to);
                    m_low[from] = std::min(m_low[from], m_low[to]);
                    continue;    
                }

                if (m_exist[to])
                    m_low[from] = std::min(m_low[from], m_time[to]);
            }

            if (m_time[from] == m_low[from])
            {
                std::vector<int> tmp;
                ++ dg.m_scc_flag;
                while (!m_record.empty())
                {
                    int vertex = m_record.top();
                    m_record.pop();
                    m_exist[vertex] = false;
                    
                    dg.m_belong[vertex] = dg.m_scc_flag;
                    tmp.push_back(vertex);
                    if (vertex == from)
                        break;    
                }
                dg.m_sccs.emplace_back(tmp.begin(), tmp.end());
            }
        }

        size_t m_timestamp{ 0 };
        std::vector<int> m_time;
        std::vector<int> m_low;
        std::vector<int> m_exist;
        std::stack<int> m_record; 

        DirectedGraph& dg;
    };

    // 双向搜索：正向图搜索 4 层，反向图搜索 3 层
    class SimpleCycle
    {
    public:
        explicit SimpleCycle(DirectedGraph& graph)
            : dg(graph)
        {
            m_graph_pos.resize(dg.m_vertex_number + 1);
            m_vis_pos.resize(dg.m_vertex_number + 1);
            m_path_pos.rehash(dg.m_vertex_number + 1);

            m_graph_neg.resize(dg.m_vertex_number + 1);
            m_vis_neg.resize(dg.m_vertex_number + 1);
            m_path_neg.rehash(dg.m_vertex_number + 1);

            m_indegree.resize(dg.m_vertex_number + 1);
            m_outdegree.resize(dg.m_vertex_number + 1);
        } 

        void run(int scc_index)
        {
            std::vector<int>& scc = dg.m_sccs[scc_index];

            for (const auto& from: scc)
            {
                for (const auto& to : dg.m_graph[from])
                {
                    if (dg.m_belong[from] == dg.m_belong[to])
                    {
                        m_graph_pos[from].push_back(to); 
                        m_graph_neg[to].push_back(from); 

                        ++ m_indegree[to];
                        ++ m_outdegree[from];
                    }
                }
            }

            std::sort(scc.begin(), scc.end(),
            [this](int lhs, int rhs)
            {
                return (m_indegree[lhs] * m_outdegree[lhs]) > 
                       (m_indegree[rhs] * m_outdegree[rhs]); 
            });
            
            for (int i = 0; i < scc.size(); ++ i)
            {
                // 起点
                int start = scc[i];

                std::vector<int> tmp_pos;
                search_pos(start, tmp_pos);
                std::vector<int> tmp_neg;
                search_neg(start, tmp_neg);

                // 对比正向边与反向边寻找环
                for (const auto& item : m_path_pos)
                {
                    if (m_path_neg[item.first].empty())
                        continue;    

                    const auto& pos = item.second;
                    const auto& neg = m_path_neg[item.first];
                    for (const auto& lhs : pos)
                    {
                        for (const auto& rhs : neg)
                        {
                            if (rhs.size() <= 2)     
                                continue;

                            int len = lhs.size() + rhs.size() - 2;
                            if (len < 3)
                                continue;

                            if (is_repeat(lhs, rhs))
                                continue;

                            std::vector<int> cycle(lhs.begin(), lhs.end());
                            cycle.insert(cycle.end(), 
                                         ++ rhs.rbegin(), -- rhs.rend());
                            dg.m_cycles.emplace_back(cycle.begin(), cycle.end());
                        }    
                    }
                }

                // 与搜索点相关的环已经处理完毕，移除该点
                dg.m_belong[start] = -1;  

                // 清理现场
                m_path_pos.clear();
                m_path_neg.clear();
            }

            for (auto& item : m_indegree)
                item = 0;
            for (auto& item : m_indegree)
                item = 0;
        }
    private:  
        // 正向图的搜索：4 层
        void search_pos(int from, std::vector<int>& path)
        {
            // 该节点已经被废弃
            if (dg.m_belong[from] == -1)
                return;

            path.push_back(from);
            m_vis_pos[from] = true;
            m_path_pos[path.back()].emplace_back(path.begin(), path.end());

            if (path.size() <= 4)
            {
                for (const auto& to : m_graph_pos[from])
                {
                    if (m_vis_pos[to]) 
                        continue;
                    search_pos(to, path);
                }
            }

            path.pop_back();
            m_vis_pos[from] = false;
        }

        // 反向图的搜索：3 层
        void search_neg(int from, std::vector<int>& path)
        {
            // 该节点已经被废弃
            if (dg.m_belong[from] == -1)
                return;

            path.push_back(from);     
            m_vis_neg[from] = true;
            m_path_neg[path.back()].emplace_back(path.begin(), path.end());
            
            if (path.size() <= 3)
            {
                for (const auto& to : m_graph_neg[from])
                {
                    if (m_vis_neg[to])
                        continue;
                    search_neg(to, path);    
                }
            }

            path.pop_back();
            m_vis_neg[from] = false;
        }

        // 是否存在重复元素
        // 判定区间：
        //      lhs -> begin, end
        //      rhs -> begin + 1, end - 1
        bool is_repeat(const std::vector<int>& lhs, 
                       const std::vector<int>& rhs)
        {
            if (rhs.size() <= 2)
                return false;

            for (const auto& item : lhs)
            {
                for (int i = 1; i < rhs.size() - 1; ++ i)
                    if (rhs[i] == item)
                        return true;    
            }
            return false;
        }

        std::vector<std::vector<int>> m_graph_pos;  // 正向图
        std::vector<bool> m_vis_pos;
        std::unordered_map<int, std::vector<std::vector<int>>> m_path_pos;

        std::vector<std::vector<int>> m_graph_neg;  // 反向图 
        std::vector<bool> m_vis_neg;
        std::unordered_map<int, std::vector<std::vector<int>>> m_path_neg;

        std::vector<int> m_indegree;
        std::vector<int> m_outdegree;

        DirectedGraph& dg;
    };
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
