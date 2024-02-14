import numpy as np
import cProfile
from bitarray import bitarray, util

n = 100_000_000

x = np.random.choice([0, 1], size=n)
y = np.random.choice([0, 1], size=n)

with cProfile.Profile() as pr:
    z = np.dot(x, y)
pr.print_stats()

x = util.urandom(n)
y = util.urandom(n)

with cProfile.Profile() as pr:
    z = bitarray.count(x & y)
pr.print_stats()

g = np.random.rand(n)
h = np.random.rand(n)

with cProfile.Profile() as pr:
    f = np.dot(g, h)
pr.print_stats()
