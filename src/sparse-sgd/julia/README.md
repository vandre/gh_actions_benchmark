# README

## Environment settings
Julia version = 1.10.0

## Running
Under julia REPL, switch to the current directory. <br />
julia> ] # switch to `pkg` mode <br />
(@v1.10) pkg> activate . # activate the environment under the current directory <br />
julia> include("deterministic.jl") # exit `pkg` mode by pushing `backspace` and run the program <br />

`bench.jl` for benchmarking results on GitHub Actions only.
