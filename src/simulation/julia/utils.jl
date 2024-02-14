
module utils
    using JSON
    export load_array, sanity_check, load_data, load_mort
    function sanity_check(res)
        @assert res[1] ≈ 811.4434227661658 # Julia value: 811.4434227662144
        @assert res[2] ≈ 121637.93004794224 # Julia value: 121637.93004794224
        @assert res[3] ≈ 2251.0134276939007 # Julia value: 2251.0134276939007
        @assert res[500] ≈ 25461.35786435145 # Julia value: 25461.35786435145
        @assert res[1000] ≈ 812.1126188746118 # Julia value: 812.1126188746483
        println("Sanity check passed")
    end

    function load_data(scenarios=1000) 
        utilPath = abspath(dirname(@__FILE__))
        #Σ data generated with rand(Uniform(-0.02, 0.08) / 12, m)
        dataPath = joinpath(utilPath,"../../../data/simulation.json")
        return load_array(dataPath)[1:scenarios]
    end

    function load_mort()
        utilPath = abspath(dirname(@__FILE__))
        #MORT data generated with fill(0.001 / 12, τ) and MortalityTables.table("2015 VBT Smoker Distinct Male Non-Smoker ALB")
        dataPath = joinpath(utilPath,"../../../data/mort.json")
        return load_array(dataPath)
    end
    
    function load_array(filename::AbstractString)::Array{Float64}
        # Open the JSON file for reading
        file = open(filename, "r")
        # Deserialize the data from the file
        data = JSON.parse(read(file, String))
        # Close the file
        close(file)
        return data    
        # Verify that the deserialized data is a Vector{Float64}
        if typeof(data) == Array{Float64}
            return data
        else
            error("Failed to load vector $(typeof(data)) from file: $filename")
        end
    end
   
end;
