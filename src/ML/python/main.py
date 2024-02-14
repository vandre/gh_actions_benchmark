import pandas as pd
from cProfile import Profile
import statsmodels.api as sm
import statsmodels.formula.api as smf
from sklearn.model_selection import KFold, cross_val_score
from sklearn import ensemble
import torch.nn as nn

df = pd.read_csv("../../../data/housing.csv")
df.fillna(0.0, inplace = True)

formula = "MEDV ~ CRIM+ZN+INDUS+CHAS+NOX+RM+AGE+DIS+RAD+TAX+PTRATIO+B+LSTAT"

with Profile() as pr:
    model = smf.glm(formula = formula, data = df, family = sm.families.Gaussian()).fit()
pr.print_stats()

print(model.summary())

ensemble.RandomForestRegressor(random_state = 1).fit(df.drop("MEDV", axis = 1), df[["MEDV"]].values.ravel())
kf = KFold(n_splits = 5, shuffle = True, random_state = 1)
score = cross_val_score(ensemble.RandomForestRegressor(random_state = 1), df.drop("MEDV", axis = 1), df[["MEDV"]].values.ravel(), cv = kf, scoring = "neg_mean_squared_error")

model = nn.Sequential(
    nn.Linear(13, 3),
    nn.BatchNorm2d(3),
    nn.Linear(13, 3, nn.ReLU)).to('cuda:0') # assume the name of gpu device is 'cuda:0'
