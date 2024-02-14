using MLDatasets
using DataFrames
using GLM
using DecisionTree
using Flux, CUDA
using BenchmarkTools

bh = BostonHousing()
features = names(bh.features)
data = copy(bh.features)
data[:, "targets"] = Vector(bh.targets[:, "MEDV"])

@btime lm(@formula(targets ~ CRIM+ZN+INDUS+CHAS+NOX+RM+AGE+DIS+RAD+TAX+PTRATIO+B+LSTAT), data)

# train regression forest, using 2 random features, 10 trees,
# averaging of 5 samples per leaf, and 0.7 portion of samples per tree
model = build_forest(Vector(bh.targets[:, "MEDV"]), Matrix(bh.features), 2, 10, 0.7, 5)
# apply learned model
apply_forest(model, fill(1.0, 13))
# run 3-fold cross validation on regression forest, using 2 random features per split
n_subfeatures=2; n_folds=3
r2 = nfoldCV_forest(Vector(bh.targets[:, "MEDV"]), Matrix(bh.features), n_folds, n_subfeatures)

model = Chain(
  Dense(13 => 3),
  BatchNorm(3),
  Dense(3 => 1),
  relu) |> gpu
