#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <iostream> 
#include <mutex>
#include <stack>
#include <thread>
#include <unordered_map>
#include <vector>


// 转账记录最多为 28 万条，最多拥有 56 万个用户参与转账
// 即图中的节点数不超过 56 万个
const int MaxUserNumber = 6e5;


// 用途：计算有向图中符合要求的环
class MoneyFlow
{
public:
    explicit MoneyFlow(int capacity)
        : m_capacity(capacity)
    {
        m_graph.resize(MaxUserNumber);
        m_scc.reserve(MaxUserNumber);
        m_cycle.reserve(MaxUserNumber);
    }    

    // 添加一条转账记录，即向图中添加一条有向边
    void addTransfer(int ori_from, int ori_to, int money)
    {
        // 忽略 money，不进行任何处理     
        
        // 离散化处理
        if (m_b2s[ori_from] == 0)
        {
            m_b2s[ori_from] = ++ m_vertex;
            m_s2b[m_vertex] = ori_from;
        }
        if (m_b2s[ori_to] == 0)
        {
            m_b2s[ori_to] = ++ m_vertex;
            m_s2b[m_vertex] = ori_to;  
        }

        m_graph[m_b2s[ori_from]].push_back(m_b2s[ori_to]);
    }

    // 检测数据，查找符合要求的环
    void detection()
    {
        m_graph.resize(m_vertex + 1); 

        // 启动线程
        for (int i = 0; i < m_capacity; ++ i)
            m_pool.emplace_back(ThreadWorker(this));

        SCC(this)();

        // 关闭线程
        m_shutdown = true;
        m_condition.notify_all();
        for (auto& item : m_pool)
            if (item.joinable())
                item.join();

        // 将离散化的数据恢复
        for (auto& row : m_cycle)
        {
            for (auto& item : row)
                item = m_s2b[item];    
            
            auto pos = std::min_element(row.begin(), row.end());
            std::vector<int> res(pos, row.end());
            for (auto iter = row.begin(); iter != pos; ++ iter)
                res.push_back(*iter);

            row = std::move(res);
        }
        std::sort(m_cycle.begin(), m_cycle.end(), 
        [](const std::vector<int>& lhs, const std::vector<int>& rhs)
        {
            if (lhs.size() < rhs.size())
                return true;
            if (lhs.size() > rhs.size())
                return false;
            return lhs < rhs;    
        });

        std::cout << "SCC = " << m_scc_flag << std::endl;
        std::cout << "Cycle = " << m_cycle.size() << std::endl;
        for (auto& row : m_cycle)
        {
            for (auto& item : row)
                std::cout << item << " ";
            std::cout << std::endl;    
        }
    }

private:
    std::vector<std::vector<int>> m_graph;
    std::vector<std::vector<int>> m_cycle;

    // 离散化处理，区间 [1, m_vertex]
    int m_vertex{ 0 }; 
    std::unordered_map<int, int> m_b2s; // big2small
    std::unordered_map<int, int> m_s2b; // small2big
    
    // 计算强连通分量
    class SCC
    {
    public:
        explicit SCC(MoneyFlow* graph)
            : p(graph)
        {
            m_time.resize(p->m_vertex + 1);
            m_low.resize(p->m_vertex + 1);
            m_exist.resize(p->m_vertex + 1);

            p->m_belong.resize(p->m_vertex + 1);
        }

        void operator()()
        {
            for (int i = 1; i <= p->m_vertex; ++ i)
                if (m_time[i] == 0)
                    tarjan(i);    
        }

    private:     
        void tarjan(int cur)
        {
            ++ m_timestamp;     
            m_time[cur] = m_low[cur] = m_timestamp; 

            m_record.push(cur);
            m_exist[cur] = true;
            
            for (const auto& next : p->m_graph[cur])
            {
                if (m_time[next] == 0)
                {
                    tarjan(next);
                    m_low[cur] = std::min(m_low[cur], m_low[next]);    
                    continue;
                }

                if (m_exist[next])
                    m_low[cur] = std::min(m_low[cur], m_low[next]);
            }

            // 标识并存储强连通分量
            if (m_time[cur] == m_low[cur])
            {
                std::vector<int> tmp;
                
                while (!m_record.empty())
                {
                    int top = m_record.top();
                    m_record.pop();
                    m_exist[top] = false;      
                    
                    // 标识
                    p->m_belong[top] = p->m_scc_flag + 1;
                    tmp.push_back(top);

                    if (top == cur)
                        break;
                }

                if (!tmp.empty())
                {
                    // 存储
                    //p->m_scc.push_back(tmp);
                    p->m_scc.emplace_back(tmp.begin(), tmp.end());
                    ++ p->m_scc_flag;
                    p->m_condition.notify_one();    
                }
            }
        }

        int m_timestamp{ 0 }; 
        std::vector<int> m_time;
        std::vector<int> m_low;

        std::vector<bool> m_exist;
        std::stack<int> m_record;

        MoneyFlow* p;
    };
 
    // 强连通分量相关
    std::atomic_int m_processed{ 0 };
    int m_scc_flag{ 0 };
    std::vector<int> m_belong; 
    std::vector<std::vector<int>> m_scc;

    // 计算某一强连通分量中的环
    class ThreadWorker
    {
    public:
        explicit ThreadWorker(MoneyFlow* graph)
            : p(graph)
        {
            m_path.resize(p->m_vertex + 1);
            m_depth.resize(p->m_vertex + 1);
        }  

        void operator()()
        {
            while (!p->m_shutdown || p->m_processed < p->m_scc_flag)
            {
                int scc_index;     
                {
                    std::unique_lock<std::mutex> ulock(p->m_mutex);
                    if (p->m_processed >= p->m_scc_flag)     
                        p->m_condition.wait(ulock);
                    scc_index = p->m_processed;
                    ++ p->m_processed;
                }

                // 虽然上面的代码进行过判断
                // 但由于关闭线程池是会调用 notify_all
                // 所以需要进行二次判断
                if (scc_index < p->m_scc_flag)
                {
                    m_scc_index = scc_index;     
                    exec();
                }
            }
        }
    private: 
        void exec()
        {
            // 不能添加此优化...
            /*
            if (p->m_scc[m_scc_index].size() < 3)
                return;
            */
            for (auto& item : m_path)
                item = 0;
            for (auto& item : m_depth)
                item = 0;

            int root = p->m_scc[m_scc_index][0];
            m_depth[root] = 1;
            dfs(root);
        }

        void dfs(int cur)
        {
            for (const auto& next : p->m_graph[cur])
            {
                // 不属于当前强连通分量
                if (p->m_belong[cur] != p->m_belong[next])
                    continue;    

                m_path[cur] = next;
                if (!m_depth[next])
                {
                    m_depth[next] = m_depth[cur] + 1;    
                    dfs(next);
                    m_depth[next] = 0;
                    continue;
                }

                int len = m_depth[cur] - m_depth[next] + 1;
                if (len < 3 || len > 7)
                    continue;

                std::vector<int> tmp;
                for (int i = next; i != cur; i = m_path[i])
                    tmp.push_back(i);
                tmp.push_back(cur);

                std::unique_lock<std::mutex> ulock(p->m_mutex);
                p->m_cycle.push_back(tmp);
            }
        }

        int m_scc_index;
        std::vector<int> m_path;
        std::vector<int> m_depth;

        MoneyFlow* p;
    };  

    int m_capacity;
    bool m_shutdown{ false };
    std::vector<std::thread> m_pool;
    std::mutex m_mutex;
    std::condition_variable m_condition;
}; 

int main(int argc, char* argv[])
{
    int from, to, money;
    char ch;
    
    MoneyFlow flow(atoi(argv[1])); 
    while (std::cin >> from >> ch >> to >> ch >> money)
        flow.addTransfer(from, to, money);
    flow.detection();
    return 0;    
}
