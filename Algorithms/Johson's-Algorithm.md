## 资料

[视频链接](https://www.youtube.com/watch?v=johyrWospv0)



## 变量作用

###### set：存储所有 blocked 的节点

###### map：存储所有 blocked 的节点，被 unblock 时，需要跟着 unblock 的节点

###### stack：dfs 过程中的路径

###### ok: 存储当前 dfs 过程中，是否找到了环



## 算法思路

1. 对 图G 求强连通分量（过滤掉不符合条件的，比如大小小于3的）
2. 强连通分量里取一个点作为 start_vertex，然后 dfs(start_vertex, current_vertex)
   1. stack 和 set 里，把 current_vertex 塞进去
   2. 对于 current_vertex 所有能到达的点 to
      1. 如果 to == start_index
         1. ok = true，找到了一个环，把 stack 里面的内容全部复制一份到 ans 里
      2. 如果 set 里面 没有 to
         1. ok = ok || dfs(start_vertex, to)
   3. 如果 ok == true
      1. unblock(current_vertex)
   4. 否则把 current_vertex 所有能到达的点 to
      1. map[to] = current_vertex
   5. stack 里删除 current_vertex
   6. return ok
3. 在 图G 中，把所有与 start_index 有关的边删除
4. 回到 1