using Printf
using CSV, DataFrames, Query
using BenchmarkTools
using Statistics

n = 100_000_000
df = DataFrame(x = rand(n))
@btime sum(df.x)

@printf("sum of column\n")

df = DataFrame(x = rand(1:10000, n))
@btime count(==(100), df.x)
@btime unique!(df.x)
@btime quantile!(df.x, 0.25)
@btime df.x[101:200]
@btime filter(x -> x > 100, df.x)

n = 1_000_000
df_1 = DataFrame(x = rand(1:10000, n), y = rand(n))
df_2 = DataFrame(x = rand(5000:15000, n), z = rand(n))
@btime df_3 = innerjoin(df_1, df_2, on = :x)
@btime df_4 = outerjoin(df_1, df_2, on = :x)
@btime df_5 = leftjoin(df_1, df_2, on = :x)
@btime df_6 = antijoin(df_1, df_2, on = :x)
@btime combine(groupby(df_1, :x), :y => mean)

# pivot
df = CSV.File("../../../data/modelingData_full_train.csv") |> DataFrame
data_pdf_wide = unstack(df, [:id, :train_row_number], :Variable_Name, :Variable_Value)
@btime data_pdf_wide = unstack(df, [:id, :train_row_number], :Variable_Name, :Variable_Value)
@btime data_pdf_long = stack(data_pdf_wide[:, :], variable_name = "Variable_Name", value_name = "Variable_Value")
