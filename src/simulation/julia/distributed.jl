push!(LOAD_PATH, abspath(dirname(@__FILE__)))
using BenchmarkTools
using Distributed
include("utils.jl")

addprocs(4)
@everywhere using DistributedArrays

τ = 120 * 12 # policy duration
n = 10_000 # number of policies
m = 1_000 # number of scenarios
MORT = utils.load_mort()
survival = cumprod(1.0 .- MORT) # mortality decrement only
POL = (fill(0.02 / 12, n), fill(1000, n)) # charges, benefits
YIELD = 1.0 .+ utils.load_data()

res = zeros((m ÷ nworkers() + 1) * nworkers())
res = distribute(res; dist = nworkers())
@everywhere function simt(survival, YIELD, POL, MORT, n, τ)
    av = ones(n) # account value
    r = zeros(n) # accumulated deficiency
    y = YIELD
    @inbounds for j = 1:τ
        gc = av .* POL[1]
        av .-= gc
        av .*= YIELD
        r .= max.(((POL[2] .- av) .* (survival[j] * MORT[j]) .- gc) ./ y, r)
        y *= YIELD
    end
    return sum(r) # total reserve is the sum of deficiencies across all policies
end
function sima(res, survival, YIELD, POL, MORT, m, n, τ)
    @sync @distributed for k = 1:m
        res_local = localpart(res)
        res_local[(k - 1) % (m ÷ nworkers() + 1) + 1] = simt(survival, YIELD[k], POL, MORT, n, τ)
    end
end
@btime sima(res, survival, YIELD, POL, MORT, m, n, τ)

for test_index in [1,2,3,m]
    println(res[test_index])
end

utils.sanity_check(res)
