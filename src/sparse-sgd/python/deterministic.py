import pickle
import scipy
import numpy as np
import json
import math
from cProfile import Profile
from pathlib import Path
from zipfile import ZipFile

def toMatrix(filename, rows, cols):
    filepath = Path(__file__).parent / "../../../data/{}".format(filename)
    with open(filepath, "r") as f:
        data = json.load(f)
    arr_1d = np.array(data)
    arr_2d = np.reshape(arr_1d, (rows, cols))
    return arr_2d

# unzip df_csr.zip to df_csr.pickle
with ZipFile("../../../data/df_csr.zip", 'r') as zObject: 
    zObject.extractall(path="../../../data")

path = Path(__file__).parent / "../../../data/df_csr.pickle"
f = open(path, "rb")
w = pickle.load(f, encoding="latin1") #pickle is already in CSR format
X = scipy.sparse.hstack((w[:, 0:11], w[:, 12:w.shape[1]]))
y = w[:, 11]
k = 10

num_samples, num_attributes = X.shape
# V = np.random.rand(num_attributes, k)
V = toMatrix("v1.json", 101856, 10)
cross_terms = X * V
total_losses = y


def sgd_V(X, total_losses, cross_terms, v, dv,
          k=10, learning_rate=0.99, m_r=0.1, m_l=0.1):
    V = v.copy()
    delta_V = dv.copy()
    x_loss = scipy.sparse.csr_matrix.copy(X)
    currV = np.matrix.copy(V)
    x_loss = X.multiply(total_losses) / X.shape[0]
    xxl = X.multiply(X).multiply(total_losses).sum(axis=0) / X.shape[0]
    xvxl = x_loss.T * cross_terms
    for f in range(k):
        for i in range(X.shape[1]):
            V[i, f] -= learning_rate * \
                ((xvxl[i, f] - xxl[0, i] * V[i, f]) +
                 m_r * delta_V[i, f] + m_l * V[i, f])
    delta_V = currV - V
    return delta_V


# V = np.random.rand(num_attributes, k)
V = toMatrix("v.json", 101856, 10)
# delta_V = np.random.rand(num_attributes, k)
delta_V = toMatrix("dv.json", 101856, 10)

with Profile() as pr:
    result = sgd_V(X, total_losses, cross_terms, V, delta_V)
pr.print_stats()

assert math.isclose(result[0][0],-0.40268408118112986, rel_tol=1e-3) 
# Invoke again to verify sgd did not mutate its inputs
result = sgd_V(X, total_losses, cross_terms, V, delta_V)  
assert math.isclose(result[0][0],-0.40268408118112986,rel_tol=1e-3) 
print(result[0][0])
