import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.Stack;

public class Main {
    
}

class Graph {
    List<List<Integer>> G;

    List<Integer> getAllVertex() {
        return null;
    }
}

class TarjanStrongConnectedComponent {
    public List<Set<Integer>> scc(Graph graph) {
        return null;
    }
}

class AllCyclesInDirectedGraphJohnson {
    Set<Integer> blockedSet;
    Map<Integer, Set<Integer>> blockedMap;
    List<List<Integer>> allCycles;
    Stack<Integer> stack;

    public List<List<Integer>> simpleCycle(Graph graph) {
        blockedSet = new HashSet<>();
        blockedMap = new HashMap<>();
        allCycles = new ArrayList<>();
        stack = new Stack<>();
        long startIndex = 1;
        TarjanStrongConnectedComponent tarjan = new TarjanStrongConnectedComponent();
        while (startIndex <= graph.getAllVertex().size()) {
            Graph subGraph = createSubGraph(startIndex, graph);     // 搜完以后，把点删了
            List<Set<Integer>> sccs = tarjan.scc(subGraph);         // 利用 tarjan 算出所有的 SCC
            // 这里可以做优化，小于 3 的 SCC，全部忽略
            Optional<Integer> maybeLeastVertex = leastIndexScc(sccs, subGraph); // 找到所有 sccs 里面，index 最小的顶点，并且忽略所有大小等于 1 的 SCC
            if (maybeLeastVertex.isPresent()) {
                Integer leastVertex = maybeLeastVertex.get();
                blockedSet.clear();
                blockedMap.clear();
                findCyclesInSCG(leastVertex, leastVertex);
                startIndex += 1;
            } else {
                break;
            }
        }
        return allCycles;
    }

    Graph createSubGraph(long startIndex, Graph graph) {
        return null;
    }

    Optional<Integer> leastIndexScc(List<Set<Integer>> sccs, Graph subGraph) {
        return null;
    }

    void findCyclesInSCG(Integer startVertex, Integer currentVertex) {
        boolean findCycle = false;
        stack
    }
}

