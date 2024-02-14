import pandas as pd
import numpy as np
from cProfile import Profile

n = 100_000_000
df = pd.DataFrame({'x': np.random.rand(n)})
with Profile() as pr:
    df.x.sum()
pr.print_stats()

df = pd.DataFrame({'x': np.random.choice(range(1, 10000), size = n)})
with Profile() as pr:
    df.x.value_counts()[100]
pr.print_stats()

with Profile() as pr:
    df.x.drop_duplicates()
pr.print_stats()

with Profile() as pr:
    np.quantile(df.x, 0.25)
pr.print_stats()

with Profile() as pr:
    df.x[100:200]​
pr.print_stats()

with Profile() as pr:
    list(filter(lambda x: x > 100, df.x))
pr.print_stats()

n = 1_000_000
df_1 = pd.DataFrame({'x': np.random.choice(range(1, 10000), size = n), 'y': np.random.rand(n)})
df_2 = pd.DataFrame({'x': np.random.choice(range(5000, 15000), size = n), 'z': np.random.rand(n)})
with Profile() as pr:
    df_3 = df_1.merge(df_2, on = 'x', how = 'inner')
pr.print_stats()

with Profile() as pr:
    df_4 = df_1.merge(df_2, on = 'x', how = 'outer')
pr.print_stats()

with Profile() as pr:
    df_5 = df_1.merge(df_2, on = 'x', how = 'left')
pr.print_stats()

with Profile() as pr:
    df_4 = df_1.merge(df_2, on = 'x', how = 'outer', indicator = True)
    df_6 = df_4[(df_4._merge == 'left_only')].drop('_merge', axis = 1)
pr.print_stats()

with Profile() as pr:
    df_1.groupby('x').y.mean()
pr.print_stats()

# pivot
df = pd.read_csv("../../../data/modelingData_full_train.csv")
with Profile() as pr:
    data_pdf_wide = df.pivot(index = ["id", "train_row_number"], columns = ["Variable_Name"], values = ["Variable_Value"])
pr.print_stats()

col_names = [col[1] for col in data_pdf_wide.columns]
data_pdf_wide.reset_index(inplace = True)
data_pdf_wide.columns = ["id", "train_row_number"] + col_names
with Profile() as pr:
    data_pdf_long = data_pdf_wide.melt(id_vars = ["id", "train_row_number"], value_vars = col_names, var_name = "Variable_Name", value_name = "Variable_Value")
pr.print_stats()
