import numpy as np
from bitarray import bitarray, util
import pytest 

def setup():
    n = 100_000_000
    x = util.urandom(n)
    y = util.urandom(n)
    return x, y

def popcnt(b1,b2):
    return bitarray.count(b1 & b2)

def test_popcnt(benchmark,capsys):
    x, y = setup()
    with capsys.disabled(): print(f"\nRunning popcnt benchmark (Bitarray)...")
    benchmark.pedantic(popcnt, args=(x,y), iterations=5, rounds=20)
