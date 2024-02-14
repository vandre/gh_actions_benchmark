from bitarray import bitarray, util

def setup():
    n = 100_000_000
    x = util.urandom(n)
    y = util.urandom(n)
    return x, y

cdef int popcnt(b1,b2):
    return bitarray.count(b1 & b2)

def popcnt_cdef(x,y):
    return popcnt(x,y)
