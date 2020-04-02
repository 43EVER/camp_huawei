#include <iostream> 
#include <algorithm>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <stack>
#include <vector>
#include <unordered_map>


// 转账记录最多为 28 万条，即最多拥有 56 万个用户参与转账
const int MaxUserNumber = 6e5;


// 用途：计算有向图中的环
// 描述：
//     通过内部的 SCC 类计算图中的强联通分量，每计算出一个则通知线程池
// 进行并行计算。
class Graph
{
public:
    // 参数表示线程池容量
    explicit Graph(int capacity)
        : m_capacity(capacity)
    {
        // 暂时调整至最大
        // 数据读入完成后将调整至最佳大小
        m_graph.resize(MaxUserNumber);    

    }

    // 向图中添加一条有向边（进行离散化处理）
    void addEdge(int ori_from, int ori_to)
    {
        if (m_discrete[ori_from] == 0)
            m_discrete[ori_from] = ++ m_vertex;
        if (m_discrete[ori_to] == 0)
            m_discrete[ori_to] = ++ m_vertex;
        
        int from = m_discrete[ori_from];
        int to = m_discrete[ori_to];

        m_graph[from].push_back(to);
    }

    void exec()
    {
        // 调整至合适尺寸
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
            
        //std::sort(m_cycle.begin(), m_cycle.end());
        //int n = std::unique(m_cycle.begin(), m_cycle.end()) - m_cycle.begin();
        //std::cout << n << std::endl; 
        
        // debug
        std::cout << "连通分量" << std::endl;
        std::cout << m_scc.size() << std::endl;
        for (int i = 0; i < m_scc.size(); ++ i)
        {
            for (int j = 0; j < m_scc[i].size(); ++ j)
                std::cout << m_scc[i][j] << " ";
            std::cout << std::endl;     
        }
        std::cout << "环" << std::endl;
        std::cout << m_cycle.size() << std::endl;
        for (int i = 0; i < m_cycle.size(); ++ i)
        {
            for (int j = 0; j < m_cycle[i].size(); ++ j)
                std::cout << m_cycle[i][j] << " "; 
            std::cout << std::endl;
        }
    }

private:
    std::vector<std::vector<int>> m_graph;

    // 离散化处理 [1, m_vertex]
    int m_vertex{ 0 };
    std::unordered_map<int, int> m_discrete;

    // 强连通分量的处理结果
    int m_scc_flag{ 0 };
    std::vector<int> m_belong;
    std::vector<std::vector<int>> m_scc;
    // 待处理的强连通分量的索引
    std::atomic_int m_process{ 0 };

    // 有向图中存在的环
    std::vector<std::vector<int>> m_cycle;

    // 线程相关
    int m_capacity;
    bool m_shutdown{ false };
    std::vector<std::thread> m_pool;
    std::mutex m_mutex;
    std::condition_variable m_condition;

    // 计算图中的强连通分量
    class SCC 
    {
        friend class Graph;
    public:
        explicit SCC(Graph* graph)
            : p(graph)
        {
            m_rank.resize(p->m_vertex + 1);
            m_low.resize(p->m_vertex + 1);
            m_exist.resize(p->m_vertex + 1);

            p->m_belong.resize(p->m_vertex + 1);
        } 

        void operator()()
        {
            for (int i = 1; i <= p->m_vertex; ++ i)
                if (m_rank[i] == 0)
                    tarjan(i);
        }

    private:
        void tarjan(int cur)
        {
            ++ m_timestamp;
            m_rank[cur] = m_low[cur] = m_timestamp;            
            m_record.push(cur);
            m_exist[cur] = true;

            for (const auto& next : p->m_graph[cur])
            {
                if (m_rank[next] == 0)
                {
                    tarjan(next);
                    m_low[cur] = std::min(m_low[cur], m_low[next]);
                    continue;
                }
                if (m_exist[next])
                    m_low[cur] = std::min(m_low[cur], m_low[next]);
            }

            // 对某一强连通分量进行标识，并存储
            if (m_rank[cur] == m_low[cur])
            {
                std::vector<int> tmp;

                while (!m_record.empty())
                {
                    int top = m_record.top();
                    m_record.pop();

                    m_exist[top] = false;
                    p->m_belong[top] = p->m_scc_flag + 1; 
                    tmp.push_back(top);
                       
                    if (top == cur)
                        break; 
                }

                if (!tmp.empty())
                {
                    p->m_scc.push_back(tmp);
                    ++ p->m_scc_flag;
                    p->m_condition.notify_one();
                }
            }
        }

        int m_timestamp{ 0 };
        // 节点的访问次序，即时间
        std::vector<int> m_rank;
        std::vector<int> m_low;
        // 节点是否处于栈中
        std::vector<bool> m_exist;
        std::stack<int> m_record;

        Graph* p; 
    };

    // 计算某一强连通分量中的环
    class ThreadWorker
    {
    public:
        explicit ThreadWorker(Graph* graph)
            : p(graph)
        {
            m_path.resize(p->m_vertex + 1);
            m_dep.resize(p->m_vertex + 1);
        } 

        void operator()()
        {
            while (!p->m_shutdown)
            {
                // 强连通分量的索引
                int index;
                {
                    std::unique_lock<std::mutex> ulock(p->m_mutex);
                    if (p->m_process >= p->m_scc_flag)
                        p->m_condition.wait(ulock);  
                    index = p->m_process;
                    ++ p->m_process;
                }
                
                // 虽然上面的代码进行过判断
                // 但关闭线程池时会调用 notify_all
                // 所以需要进行二次判断 
                if (index < p->m_scc_flag)
                {
                    m_index = index;
                    task();
                }
            }
        }

    private:
        void init()
        {
            for (auto& item : m_path)
                item = 0;
            for (auto& item : m_dep)
                item = 0;     
        }

        void task()
        {
            if (p->m_scc[m_index].size() <= 2)
                return;

            init();
            int cur = p->m_scc[m_index][0];
            m_dep[cur] = 1;
            dfs(cur);
        }

        void dfs(int cur)
        {
            for (const auto& next : p->m_graph[cur])
            {
                // 不属于当前强连通分量
                if (p->m_belong[next] != p->m_belong[cur])
                    continue;

                m_path[cur] = next; 
                if (!m_dep[next])
                {
                    m_dep[next] = m_dep[cur] + 1;
                    dfs(next);
                    m_dep[next] = 0; 
                    continue;   
                }
                    
                int len = m_dep[cur] - m_dep[next] + 1;
                if (len < 3 || len > 7)
                    continue;

                std::vector<int> tmp;
                for (int i = next; i != cur; i = m_path[i])
                    tmp.push_back(i);
                tmp.push_back(cur);

                auto pos = std::min_element(tmp.begin(), tmp.end());
                std::vector<int> res(pos, tmp.end());
                for (auto iter = tmp.begin(); iter != pos; ++ iter)
                    res.push_back(*iter);

                std::unique_lock<std::mutex> ulock(p->m_mutex);
                p->m_cycle.push_back(res);
            }
        }

        // 强连通分量的索引 
        int m_index;
        // 路径
        std::vector<int> m_path;
        // 节点深度（剪枝）
        std::vector<int> m_dep;

        Graph* p; 
    };
};

int main(int argc, char* argv[])
{
    int from, to, money;
    char ch;
    Graph graph(1);
    // while (std::cin >> from >> to >> money)
    while (std::cin >> from >> ch >> to >> ch >> money)
        graph.addEdge(from, to);
    graph.exec();
    return 0;    
}
