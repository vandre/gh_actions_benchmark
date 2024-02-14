macro printobj(obj)
    return quote
        local io = IOBuffer()
        show(io, "text/plain", $obj)
        println(String(take!(io)))
    end
end

macro savebench(t, filename)
    return quote
        local data = Dict("mean_ns"=> mean($t).time, "min_ns"=> minimum($t).time, 
        "max_ns"=> maximum($t).time, "med_ns"=>median($t).time)
        @printobj $t
        println("Saving benchmark to $(isabspath($filename) ? 
        $filename : joinpath(pwd(),$filename)).json")
        open("$($filename).json", "w") do f JSON.print(f, data) end
    end
end

