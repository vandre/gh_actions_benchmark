using LinearAlgebra
using BenchmarkTools

n = 100_000_000
x = BitVector(rand(Bool, n))
y = BitVector(rand(Bool, n))

@show "Native Bitwise-And"
@btime z = sum(x .& y)

@show "LinearAlgebra invocation"
@btime z = LinearAlgebra.dot(x, y)

@show "Processed $n bits"

g = rand(n)
h = rand(n)
@btime f = LinearAlgebra.dot(g, h)

@show "Processed $n numbers"
