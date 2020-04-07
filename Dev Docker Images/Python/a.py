from cyaron import *


n = 2500
m = 25000

graph = Graph.DAG(n, m, loop=True)

test_data = IO(file_prefix="" ,data_id=1)
test_data.input_writeln(graph)