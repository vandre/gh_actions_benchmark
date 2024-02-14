using BenchmarkTools
using JSON
using LinearAlgebra
include("../../bench_utils.jl")

n = 100_000_000
x = BitVector(rand(Bool, n))
y = BitVector(rand(Bool, n))

function popcnt(b1,b2)
    return LinearAlgebra.dot(b1,b2)
end

t = @benchmark popcnt(x,y) samples=20

@savebench t basename(@__FILE__)
