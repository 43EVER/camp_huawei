#include <iostream>
#include <vector>
#include <stack>


class Graph
{
public:
    explicit Graph(int v)    
        : vertex(v), timestamp(0), number(0),
          g(vertex + 1), 
          rank(vertex + 1), low(vertex + 1), vis(vertex + 1), 
          color(vertex + 1)  
    {
    }

    void addEdge(int from, int to)
    {
        g[from].push_back(to);    
    }

    void debug()
    {
        for (int i = 1; i < g.size(); ++ i)
        {
            std::cout << i << " ||| ";
            for (int j = 0; j < g[i].size(); ++ j)
                std::cout << g[i][j] << " "; 
            std::cout << std::endl;
        }
    }

    void calu_scc()
    {
        for (int i = 1; i <= vertex; ++ i)
        {
            if (rank[i] == 0)
                tarjan(i);
        }

        std::cout << "强联通分量的个数 = " << number << std::endl;
        scc.resize(number);
        for (int i = 1; i <= vertex; ++ i)   
            scc[color[i] - 1].push_back(i);

        for (int i = 0; i < scc.size(); ++ i)
        {
            for (int j = 0; j < scc[i].size(); ++ j)
                std::cout << scc[i][j] << " ";
            std::cout << std::endl;    
        }
    }

private:
    void tarjan(int cur)
    {
        ++ timestamp;
        rank[cur] = low[cur] = timestamp;
        sta.push(cur);
        vis[cur] = true; 

        for (int i = 0; i < g[cur].size(); ++ i)
        {
            int next = g[cur][i];
            if (rank[next] == 0)
            {
                tarjan(next);
                low[cur] = std::min(low[cur], low[next]);
                continue;
            }
            if (vis[next])
                low[cur] = std::min(low[cur], low[next]);
        }

        if (rank[cur] == low[cur])
        {
            ++ number;
            while (!sta.empty())
            {
                int top = sta.top();
                sta.pop();
                vis[top] = false;

                color[top] = number;

                if (top == cur)
                    break;     
            } 
        }
    }

    // 顶点数
    int vertex;
    // 邻接表
    std::vector<std::vector<int>> g;

    // 时间
    int timestamp; 
    // 节点的访问次序：即时间
    std::vector<int> rank; 
    std::vector<int> low;
    // 节点是否处于栈中
    std::vector<bool> vis;
    std::stack<int> sta;
    
    // 节点所属的强联通分量
    int number;
    std::vector<int> color;

    // 图的强联通分量
    std::vector<std::vector<int>> scc;
};


int main(int argc, char* argv[])
{
    int n, m;
    std::cin >> n >> m;
    
    Graph graph(n);
    for (int i = 0; i < m; ++ i)
    {
        int from, to;
        std::cin >> from >> to;
        graph.addEdge(from, to);
    }
    graph.debug();
    graph.calu_scc();
    return 0;   
}
