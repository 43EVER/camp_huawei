from cyaron import *


n = 5000
m = 8000

graph = Graph.DAG(n, m, loop=True)

test_data = IO(file_prefix="" ,data_id=1)
test_data.input_writeln(graph)