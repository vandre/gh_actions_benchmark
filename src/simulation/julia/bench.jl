push!(LOAD_PATH, abspath(dirname(@__FILE__)))
using JSON
using BenchmarkTools
using Distributed
include("utils.jl")
include("../../bench_utils.jl")

ENV["SCENARIOS"] = 1_000
addprocs(16, exeflags="--project")
@everywhere using DistributedArrays

τ = 120 * 12 # policy duration
n = 10_000 # number of policies
m = something(tryparse(Int64, ENV["SCENARIOS"]), 1_000) # number of scenarios # number of scenarios
MORT = utils.load_mort()
survival = cumprod(1.0 .- MORT) # mortality decrement only
POL = (fill(0.02 / 12, n), fill(1000, n)) # charges, benefits
YIELD = 1.0 .+ utils.load_data(m)

res = zeros((m ÷ nworkers() + 1) * nworkers())
res = distribute(res; dist = nworkers())
@everywhere function simt(YIELD, POL, survival, n, τ)
    av = ones(n) # account value
    gc = zeros(n) # policy charge
    r = zeros(n) # accumulated deficiency
    y = YIELD
    @inbounds @fastmath for j = 1:τ
        @inbounds @fastmath for k = 1:n
            gc[k] = av[k] * POL[1][k]
            av[k] -= gc[k]
            av[k] *= YIELD
            r[k] = max(((POL[2][k] - av[k]) * survival[j] - gc[k]) / y, r[k])
        end
        y *= YIELD
    end
    return sum(r) # total reserve is the sum of deficiencies across all policies
end
function sima(res, survival, YIELD, POL, MORT, m, n, τ)
    @sync @distributed for k = 1:m
        res_local = localpart(res)
        res_local[(k - 1) % (m ÷ nworkers() + 1) + 1] = simt(YIELD[k], POL, survival .* MORT, n, τ)
    end
end
println("Running $m scenarios...")
t = @benchmark sima(res, survival, YIELD, POL, MORT, m, n, τ) samples=5 seconds=60

for test_index in [1,2,3,m]
    println(res[test_index])
end

@savebench t basename(@__FILE__)
