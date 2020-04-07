import networkx as nx
import matplotlib.pyplot as plt

# Create Directed Graph
G=nx.DiGraph()

# Add a list of nodes:
G.add_nodes_from(["a","b","c","d","e"])

# Add a list of edges:
G.add_edges_from([("a","b"),("b","c"), ("c","a"), ("b","d"), ("d","e"), ("e","a")])

#Return a list of cycles described as a list o nodes
list(nx.simple_cycles(G))