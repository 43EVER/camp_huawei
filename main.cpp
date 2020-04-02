#include <iostream>
#include <stack>
#include <vector>
#include <unordered_map>


// 转账数量最多为 28 万，即最多拥有 56 万个用户参与转账
const int MaxUserNumber = 6e5;

class Graph
{
public:
    Graph()
    {
        // 暂时调整至最大
        // 数据读入完成后将调整至最佳大小
        graph.resize(MaxUserNumber);    
    }

    void addEdge(int ori_from, int ori_to)
    {
        if (discrete[ori_from] == 0)
            discrete[ori_from] = ++ vertex;
        if (discrete[ori_to] == 0)
            discrete[ori_to] = ++ vertex;
        
        int from = discrete[ori_from];
        int to = discrete[ori_to];

        graph[from].push_back(to);
    }

    void exec()
    {
        SCC calu_scc(this);
        calu_scc();
        
        belong = std::move(calu_scc.belong);
        scc = std::move(calu_scc.scc); 

        // debug
        /*
        for (int i = 0; i < scc.size(); ++ i)
        {
            for (int j = 0; j < scc[i].size(); ++ j)
                std::cout << scc[i][j] << " ";
            std::cout << std::endl;    
        }
        */ 

        Cycle calu_cycle(this);
        calu_cycle();
    }
private:
    // 离散化处理 [1, vertex]
    int vertex = 0;
    std::unordered_map<int, int> discrete;
    std::vector<std::vector<int>> graph;

    // SCC 的处理结果
    std::vector<int> belong;
    std::vector<std::vector<int>> scc;

    // 计算图中的强连通分量
    class SCC 
    {
        friend class Graph;
    public:
        explicit SCC(Graph* graph)
            : p(graph)
        {
            rank.resize(p->vertex + 1);
            low.resize(p->vertex + 1);
            exist.resize(p->vertex + 1);
            belong.resize(p->vertex + 1);
        } 

        void operator()()
        {
            for (int i = 1; i <= p->vertex; ++ i)
                if (rank[i] == 0)
                    tarjan(i);

            scc.resize(number);
            for (int i = 1; i <= p->vertex; ++ i)
                scc[belong[i] - 1].push_back(i);
        }

    private:
        void tarjan(int cur)
        {
            ++ timestamp;
            rank[cur] = low[cur] = timestamp;            
            record.push(cur);
            exist[cur] = true;

            for (const auto& next : p->graph[cur])
            {
                if (rank[next] == 0)
                {
                    tarjan(next);
                    low[cur] = std::min(low[cur], low[next]);
                    continue;
                }
                if (exist[next])
                    low[cur] = std::min(low[cur], low[next]);
            }

            if (rank[cur] == low[cur])
            {
                ++ number;
                while (!record.empty())
                {
                    int top = record.top();
                    record.pop();

                    exist[top] = false;
                    belong[top] = number; 
                       
                    if (top == cur)
                        break; 
                }
            }
        }

        int timestamp = 0;
        // 节点的访问次序，即时间
        std::vector<int> rank;
        std::vector<int> low;
        // 节点是否处于栈中
        std::vector<bool> exist;
        std::stack<int> record;

        // 节点所属的强联通分量
        int number = 0;
        std::vector<int> belong; 

        // 图的强连通分量
        std::vector<std::vector<int>> scc;

        Graph* p; 
    };

    // 计算 DAG 中的环
    class Cycle
    {
        friend class Graph;
    public:
        explicit Cycle(Graph* graph)
            : p(graph)
        {
            record.resize(p->vertex + 1);
            exist.resize(p->vertex + 1);
            vis.resize(p->vertex + 1);
        }

        void operator()()
        {
            for (int i = 1; i <= p->vertex; ++ i)
                if (!vis[i])
                    dfs(i);
        }
    private:
        void dfs(int cur)
        {
            vis[cur] = true;
            record[++ top] = cur;
            exist[cur] = true;

            for (const auto& next : p->graph[cur])
            {
                if (!exist[next])
                    dfs(next);
                else
                {
                    int beg = top;
                    while (record[beg] != next)
                        -- beg;
                    
                    std::cout << "find ";
                    for (int i = beg; i <= top; ++ i)
                        std::cout << record[i] << " ";
                    std::cout << std::endl;      
                }
            }
            exist[cur] = false;
            -- top;
        }

        // 数组栈
        int top = 0; 
        std::vector<int> record;
        std::vector<bool> exist;
        
        std::vector<int> vis;

        Graph* p;
    }; 
};

int main(int argc, char* argv[])
{
    int from, to, money;
    char ch;
    Graph graph;
    while (std::cin >> from >> ch >> to >> ch >> money)
        graph.addEdge(from, to);
    graph.exec();
    return 0;    
}
