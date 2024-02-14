using JSON
using PyCall
using SparseArrays
using BenchmarkTools
using ZipFile

@pyimport pickle
@pyimport scipy

function toMatrix(filename::AbstractString,rows,cols)
    utilPath = abspath(dirname(@__FILE__))
    filePath = joinpath(utilPath,"../../../data/$filename")
    file = open(filePath, "r")
    data::Array{Float64} = JSON.parse(read(file, String))
    close(file)
    return reshape(data,cols,rows) |> permutedims    
end

# unzip df_csr.zip and write to df_csr.pickle
f = open("../../../data/df_csr.pickle", "w")
write(f, read(ZipFile.Reader("../../../data/df_csr.zip").files[1], String))
close(f)

f = py"""open("../../../data/df_csr.pickle", "rb")"""
data = pickle.load(f, encoding = "latin1")
w = sparse(data.nonzero()[1].+1, data.nonzero()[2].+1, data.data)
X = hcat(w[:, 1:11], w[:, 13:end])
y = Vector{Float32}(w[:, 12])
κ = 10

num_samples, num_attributes = size(X)
#V = rand(num_attributes, κ)
V = toMatrix("v1.json",101856,10)
cross_terms = X * V # slower
total_losses = y

function sgd_V(X, total_losses, cross_terms, v, dv, κ = 10, α = 0.99, γ = 0.1, λᵥ = 0.1)
    V = copy(v)
    ΔV = copy(dv)
    x_loss = copy(X)
    xxll = copy(X)
    currV = copy(V)
    x_loss.nzval .*= total_losses[X.rowval]; x_loss.nzval ./= X.m
    xxll .*= xxll; xxll.nzval .*= total_losses[X.rowval]; xxll.nzval ./= X.m; xxl = sum(xxll, dims=1)
    xvxl = x_loss' * cross_terms # slower
    @inbounds for f in 1:κ
        @inbounds for i in 1:X.n
            V[i, f] -= α * ((xvxl[i, f] - xxl[i] * V[i, f]) + γ * ΔV[i, f] + λᵥ * V[i, f])
        end
    end
    ΔV .= currV .- V
end


#V = rand(X.n, κ)
V = toMatrix("v.json",101856,10)
#ΔV = rand(X.n, κ)
ΔV = toMatrix("dv.json",101856,10)
#JIT
result = sgd_V(X, total_losses, cross_terms, V, ΔV)
@time result = sgd_V(X, total_losses, cross_terms, V, ΔV)
@assert result[1,1] ≈ -0.40286116029480534
result = sgd_V(X, total_losses, cross_terms, V, ΔV) #Invoke again to verify sgd did not mutate its inputs
@assert result[1,1] ≈ -0.40286116029480534
