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
    // 生产者：主线程负责计算强连通分量
    // 消费者：其他线程负责计算强连通分量的环
    // 参数 capacity 表示线程池的容量大小
    explicit MoneyFlow(int capacity)
        : m_capacity(capacity)
    {
        // 暂设为最大值，记录添加完成后再调整至最佳大小
        m_graph.resize(MaxUserNumber);
        // 预设大小，防止向 vector 中添加元素时导致元素的搬运
        m_scc.reserve(MaxUserNumber); 
        m_cycle.reserve(MaxUserNumber);
    }

    // 添加一条转账记录，即向图中添加一条有向带权边
    void addTransfer(int from, int to, int money)
    {
        // 离散化处理
        if (m_b2s[from] == 0)
        {
            m_b2s[from] = ++ m_discrete_value;    
            m_s2b[m_discrete_value] = from;
        }
        if (m_b2s[to] == 0)
        {
            m_b2s[to] = ++ m_discrete_value;    
            m_s2b[m_discrete_value] = to;
            //to = m_discrete_value;
        }

        // 添加有向边
        m_graph[m_b2s[from]].push_back(m_b2s[to]);
    }

    // 检测转账数据，查找符合要求的环
    void detection()
    {
        // 图中的节点数量等同于参与转账的用户数量
        // 区间：[1, m_vertex]
        m_vertex = m_discrete_value; 
    
        // 调整数据结构的大小
        m_graph.resize(m_vertex + 1);
        m_belong.resize(m_vertex + 1);

        // 启动线程池
        for (int i = 0; i < m_capacity; ++ i)
            m_thread_pool.emplace_back(ThreadWorker(this));

        // 生产者
        SCC1(this)();

        // 关闭线程池
        m_shutdown = true;
        m_condition.notify_all();
        for (auto& item : m_thread_pool)
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
        
        // 排序
        std::sort(m_cycle.begin(), m_cycle.end(),
        [](const std::vector<int>& lhs, const std::vector<int>& rhs)
        {
            if (lhs.size() < rhs.size())
                return true;
            if (lhs.size() > rhs.size())
                return false; 
            return lhs < rhs;
        });

        // 去重
        auto iter = std::unique(m_cycle.begin(), m_cycle.end());
        m_cycle.erase(iter, m_cycle.end());

        //std::cout << "SCC = " << m_scc.size() << std::endl;
        //std::cout << "Cycle = " << m_cycle.size() << std::endl;
        std::cout << m_cycle.size() << std::endl;
        for (auto& row : m_cycle)
        {
            for (auto& item : row)
                std::cout << item << " ";
            std::cout << std::endl;
        }
    }

private:
    int m_vertex;                           // 节点数量
    int m_capacity;                         // 线程池的容量大小
    
    // 离散化，区间 [1, m_discrete_value]
    int m_discrete_value{ 0 };
    std::unordered_map<int, int> m_b2s;     // 将大范围离散至小范围
    std::unordered_map<int, int> m_s2b;     // 将小范围恢复至大范围

    std::vector<std::vector<int>> m_graph;  // 邻接表
    std::vector<std::vector<int>> m_scc;    // 强连通分量
    std::vector<std::vector<int>> m_cycle;  // 环

    // 强连通分量标记
    int m_scc_flag{ 0 };
    std::vector<int> m_belong;              // 节点所属的强连通分量

    // 线程相关
    int m_consumed{ 0 };                    // 已经消费的数量
    bool m_shutdown{ false };               // 线程池是否关闭
    std::vector<std::thread> m_thread_pool; // 线程池
    std::mutex m_mutex;                     // 互斥量
    std::condition_variable m_condition;    // 条件变量
    
    // 计算强连通分量
    class SCC1
    {
    public:
        explicit SCC1(MoneyFlow* graph)
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
                    tmp.push_back(top);

                    // 标记
                    p->m_belong[top] = p->m_scc_flag + 1; 

                    if (top == cur)
                        break;
                }

                //if (tmp.size() >= 3)
                if (true)
                {
                    ++ p->m_scc_flag;
                    // 存储
                    p->m_scc.emplace_back(tmp.begin(), tmp.end());
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
    // 计算强连通分量
    class SCC
    {
    public:
        explicit SCC(MoneyFlow* mf)
            : p(mf)
        {
            m_time.resize(p->m_vertex + 1);     
            m_low.resize(p->m_vertex + 1);
            m_exist.resize(p->m_vertex + 1);
        }

        void operator()()
        {
            for (int i = 1; i <= p->m_vertex; ++ i)
                if (m_time[i] == 0)
                    tarjan(i);      
        }

    private:
        void tarjan(int from)
        {
            ++ m_timestamp;
            m_time[from] = m_low[from] = m_timestamp;
                 
            m_record.push(from);
            m_exist[from] = true;

            for (const auto& to : p->m_graph[from])
            {
                if (m_time[to] == 0)
                {
                    tarjan(to);
                    m_low[from] = std::min(m_low[from], m_low[to]);
                    continue;
                }

                if (m_exist[to])
                    m_low[from] = std::min(m_low[from], m_low[to]);
            }

            // 1. 标识强连通分量
            // 2. 存储强连通分量
            // 3. 通知其他线程进行计算
            if (m_time[from] == m_low[from])
            {
                ++ p->m_scc_flag;
                std::vector<int> scc;
                while (!m_record.empty())
                {
                    int top = m_record.top();
                    m_record.pop();
                    m_exist[top] = false;
                    
                    scc.push_back(top);     
                    p->m_belong[top] = p->m_scc_flag;

                    if (top == from)
                        break;
                }

                // 当强连通分量中节点数小于 3 时，不满足规范
                if (scc.size() >= 3)
                {
                    p->m_scc.push_back(scc);
                    p->m_condition.notify_one();  
                }
            }
        }

        int m_timestamp{ 0 };       // 时间戳，标记节点的访问次序
        std::vector<int> m_time;    // 节点访问次序
        std::vector<int> m_low;
        std::vector<int> m_exist;   // 节点是否位于栈中
        std::stack<int> m_record;   // 可能构成强连通分量的点

        MoneyFlow* p; 
    }; 

    class ThreadWorker
    {
    public:
        explicit ThreadWorker(MoneyFlow* mf)
            : p(mf)
        {
            m_path.resize(p->m_vertex + 1);
            m_depth.resize(p->m_vertex + 1);
        }
    
        void operator()()
        {
            // 消费，直至线程池被关闭且产品无剩余
            while (p->m_consumed < p->m_scc.size() || !p->m_shutdown)
            {
                std::unique_lock<std::mutex> ulock(p->m_mutex);
                p->m_condition.wait(ulock, [this]
                {
                    // 存在可消费的产品
                    if (p->m_consumed < p->m_scc.size())
                        return true; 
                    return p->m_shutdown;
                });

                // 唤醒条件可能是由于线程池关闭时的 notify_all 信号引起
                // 此时可能不存在可消费的产品
                // 所以需要进行二次判断
                if (p->m_consumed < p->m_scc.size())
                {
                    m_process = p->m_consumed ++; 
                    ulock.unlock();

                    exec();
                } 
            }
        }
    private:
        void exec()
        {
            // 当强连通分量中的节点数小于 3 时不进行计算
            if (p->m_scc[m_process].size() < 3)
                return; 
            std::cout << m_process << " " << p->m_scc.size() << std::endl;
            // 初始化相关数据结构
            for (auto& item : m_path)
                item = 0;
            for (auto& item : m_depth)
                item = 0;

            int root = p->m_scc[m_process][0];
            m_depth[root] = 1;

            search(root);
        }

        void search(int from)
        {
            for (const auto& to : p->m_graph[from])
            {
                // 不属于当前强连通分量
                if (p->m_belong[from] != p->m_belong[to])
                    continue;    

                m_path[from] = to;
                if (!m_depth[to])
                {
                    m_depth[to] = m_depth[from] + 1;
                    search(to);
                    m_depth[to] = 0;
                    continue;    
                }

                int len = m_depth[from] - m_depth[to] + 1;
                // 环的长度不满足要求
                if (len < 3 || len > 7)
                    continue;

                // 提取环的路径
                std::vector<int> cycle;
                for (int i = to; i != from; i = m_path[i])
                    cycle.push_back(i);
                cycle.push_back(from);

                std::unique_lock<std::mutex> ulock(p->m_mutex);
                p->m_cycle.push_back(cycle);
            }
        }
        
        int m_process;              // 正在处理的产品索引
        std::vector<int> m_path;    // 访问路径
        std::vector<int> m_depth;   // 节点深度

        MoneyFlow* p;    
    };
};


int main(int argc, char* argv[])
{
    std::ios::sync_with_stdio(false);

    int from, to, money;
    char ch;

    MoneyFlow mf(atoi(argv[1]));
    while (std::cin >> from >> ch >> to >> ch >> money)
        mf.addTransfer(from, to, money);
    mf.detection();
    return 0;
}
