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
        m_available.resize(m_vertex_number + 1, true);
        //m_in_degree.resize(m_vertex_number + 1);
        //m_out_degree.resize(m_vertex_number + 1);

        m_graph_pos.resize(m_vertex_number + 1);
        m_graph_neg.resize(m_vertex_number + 1);
    }

    void addEdge(int from, int to)
    {
        m_graph_pos[from].push_back(to);
        m_graph_neg[to].push_back(from); 

        //++ m_in_degree[to];
        //++ m_out_degree[from];
    }

    std::vector<std::vector<int>>& exec()
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < 4; ++ i)
            threads.emplace_back(ThreadWorker(*this));
        for (auto& thread : threads)
            if (thread.joinable())
                thread.join();

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
    
    class ThreadWorker
    {
    public:
        explicit ThreadWorker(DirectedGraph& graph)
            : dg(graph)
        {
            m_vis_pos.resize(dg.m_vertex_number + 1, false);
            m_path_pos.rehash(dg.m_vertex_number + 1);
            m_vis_neg.resize(dg.m_vertex_number + 1, false);
            //m_path_neg.rehash(dg.m_vertex_number + 1);
        } 

        void operator()()
        {
            while (dg.m_progress <= dg.m_vertex_number)
            {
                std::unique_lock<std::mutex> ulock(dg.m_mutex);
                int index = ++ dg.m_progress;     
                ulock.unlock();

                if (index > dg.m_vertex_number)
                    break;

                run(index);
            }

            std::unique_lock<std::mutex> ulock(dg.m_mutex);
            for (auto& cycle : m_internal_cycles)
                dg.m_cycles.emplace_back(std::move(cycle));
        }

    private:
        void run(int index)
        {
            std::vector<int> tmp_pos_path;     
            search_pos(index, tmp_pos_path);
            std::vector<int> tmp_neg_path;
            search_neg(index, tmp_neg_path);

            /*
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
                        m_internal_cycles.emplace_back(cycle.begin(), cycle.end());
                    }    
                }
            }
            */

            m_path_pos.clear();
            //m_path_neg.clear();
            std::unique_lock<std::mutex> ulock(dg.m_mutex);
            dg.m_available[index] = false;
            ulock.unlock();
        }

        void search_pos(int from, std::vector<int>& path)
        {
            if (!dg.m_available[from])
                return;

            path.push_back(from);
            m_vis_pos[from] = true;
            m_path_pos[from].emplace_back(path.begin(), path.end());
            
            if (path.size() <= 4)
            {
                for (const auto& to : dg.m_graph_pos[from])
                {
                    if (m_vis_pos[to])
                        continue;
                    search_pos(to, path);   
                }
            }

            path.pop_back();
            m_vis_pos[from] = false;
        }

        void search_neg(int from, std::vector<int>& path)
        {
            if (!dg.m_available[from])
                return;
            
            path.push_back(from);
            m_vis_neg[from] = true;
            //m_path_neg[from].emplace_back(path.begin(), path.end());
            
            if (!m_path_pos[from].empty())
            {
                const auto& rhs = path;
                for (const auto& lhs : m_path_pos[from])     
                {
                    int len = lhs.size() + rhs.size() - 2;
                    if (len < 3)
                        continue;

                    if (is_repeat(lhs, rhs))
                        continue;

                    std::vector<int> cycle(lhs.begin(), lhs.end());
                    cycle.insert(cycle.end(), 
                                 ++ rhs.rbegin(), -- rhs.rend());
                    m_internal_cycles.emplace_back(cycle.begin(), cycle.end());
                }
            }

            if (path.size() <= 3)
            {
                for (const auto& to : dg.m_graph_neg[from])
                {
                    if (m_vis_neg[to])
                        continue;
                    search_neg(to, path); 
                }
            }

            path.pop_back();
            m_vis_neg[from] = false;
        }

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

        std::vector<bool> m_vis_pos;
        std::unordered_map<int, std::vector<std::vector<int>>> m_path_pos;
        std::vector<bool> m_vis_neg;
        //std::unordered_map<int, std::vector<std::vector<int>>> m_path_neg;

        std::vector<std::vector<int>> m_internal_cycles;

        DirectedGraph& dg; 
    };

    size_t m_vertex_number;
    std::vector<bool> m_available;
    //std::vector<int> m_in_degree;
    //std::vector<int> m_out_degree;
    
    std::vector<std::vector<int>> m_graph_pos;
    std::vector<std::vector<int>> m_graph_neg;

    std::vector<std::vector<int>> m_cycles;

    size_t m_progress{ 0 };
    std::mutex m_mutex;
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
