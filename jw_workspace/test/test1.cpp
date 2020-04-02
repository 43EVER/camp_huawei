#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <algorithm>
#include <functional>
using namespace std;

const int N = 5000 + 10;
vector<int> g[N];
int st[N],instack[N],mark[N],top;
void dfs(int u)
{
    instack[u] = true;
    mark[u] = true;//因为可能不联通，所以需要没有mark过的点，需要再次调用dfs
    st[++top] = u;
    for(int i=0;i<g[u].size(); ++i)
    {
        int v = g[u][i];
        if(!instack[v])
            dfs(v);
        else
        {

            int t;
            for(t=top;st[t]!=v;--t);//在栈中找到环的起始点
            printf("%d:",top-t+1);//这这里就能判断是奇数环还是偶数环
            for(int j=t;j<=top;++j)
                printf("%d ",st[j]);
            puts("");
        }
    }
    instack[u] = false;
    top--;
}
void init(int n)
{
    for(int i=1;i<=n;++i)
    {
        g[i].clear();
        instack[i] = mark[i] = 0;
    }
    top = 0;
}
int main()
{
    int n,m;
    while(scanf("%d%d",&n,&m)!=EOF)
    {
        init(n);
        int u,v;
        for(int i=1;i<=m;++i)
        {
            scanf("%d%d",&u,&v);
            g[u].push_back(v);
        }
        for(int i=1;i<=n;++i)
            if(!mark[i])
                dfs(i);
    }
    return 0;
}
