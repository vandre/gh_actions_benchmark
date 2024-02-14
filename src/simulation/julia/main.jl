push!(LOAD_PATH, abspath(dirname(@__FILE__)))
using Distributions
using BenchmarkTools
using MortalityTables
include("utils.jl")

τ = 120 * 12 # policy duration
n = 10_000 # number of policies
m = 1_000 # number of scenarios
MORT = utils.load_mort()
survival = cumprod(1.0 .- MORT) # mortality decrement only
POL = (fill(0.02 / 12, n), fill(1000, n)) # charges, benefits
YIELD = 1.0 .+ utils.load_data()

gc = zeros(n) # policy charge
b = zeros(n) # benefit
res = zeros(m) # reserve
function sima(gc, b, res, survival, YIELD, POL, MORT, m, n, τ)
    @inbounds for k = 1:m # number of scenarios
        r = zeros(n) # accumulated deficiency
        av = ones(n) # account value
        @inbounds for j = 1:τ # number of timepoints
            gc .= av .* POL[1]
            av .-= gc
            av .*= YIELD[k]
            b .= (POL[2] .- av) .* (survival[j] * MORT[j])
            r .= max.((b .- gc) ./ YIELD[k] ^ j, r)
        end
        res[k] = sum(r) # total reserve is the sum of deficiencies across all policies
    end
end
@btime sima(gc, b, res, survival, YIELD, POL, MORT, m, n, τ)

for test_index in [1,2,3,500,1000]
    println(res[test_index])
end

utils.sanity_check(res)
