#include <iostream>
#include <vector>
#include <stack>
#include <queue>


class Graph
{
public:
    explicit Graph(int v)
        : vertex(v)
    {
        g.resize(vertex + 1);    
        vis.resize(vertex + 1);
    } 

    void addEdge(int from, int to)
    {
        g[from].push_back(to);   
    }

    void calu_cycle()
    {
        search(1, 0); 
    }
private:
    void search(int cur, int level)
    {
        vis[cur] = true;
        for (int i = 0; i < g[cur].size(); ++ i)
        {
            int next = g[cur][i];
            if (vis[next])
            {
                // 出现了一个环
                for (const auto& item : single)
                    std::cout << item << " ";
                std::cout << std::endl;
                single.clear(); 
            }
            else
            {
                single.push_back(cur);
                search(next, level + 1);
            }
        }
    }
    
    int vertex;
    std::vector<std::vector<int>> g;  

    std::vector<bool> vis;

    std::vector<int> single;
    std::vector<std::vector<int>> res;
};

int main()
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
    graph.calu_cycle();
    return 0;    
}
