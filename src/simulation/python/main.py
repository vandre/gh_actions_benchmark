import json
import numpy as np
from cProfile import Profile
from pathlib import Path

t = 120 * 12 # policy duration
n = 10000 # number of policies
m = 1000 # number of scenarios
#MORT = np.full(t, 0.001 / 12) # temporary
path = Path(__file__).parent / "../../../data/mort.json"
with open(path, 'r') as file: MORT = np.array(json.load(file))
survival = np.cumprod(1.0 - MORT)
#sigma = np.array([np.random.uniform(-0.02, 0.08) for _ in range(m)])
path = Path(__file__).parent / "../../../data/simulation.json"
with open(path, 'r') as file: sigma = np.array(json.load(file))
POL = (np.full(n, 0.02 / 12), np.full(n, 1000))
YIELD = 1.0 + sigma

gc = np.zeros(n) # policy charge
b = np.zeros(n) # benefit
res = np.zeros(m) # reserve
def sima(gc, b, res, survival, YIELD, POL, MORT, m, n, t):
    for k in range(m): # number of scenarios
        r = np.zeros(n) # accumulated deficiency
        av = np.ones(n) # account value
        for j in range(t): # number of timepoints
            gc = av * POL[0]
            av -= gc
            av *= YIELD[k]
            b = (POL[1] - av) * (survival[j] * MORT[j])
            r = np.maximum((b - gc) / (YIELD[k] ** (j + 1)), r)
        res[k] = sum(r) # total reserve is the sum of deficiencies across all policies

with Profile() as pr:
    sima(gc, b, res, survival, YIELD, POL, MORT, m, n, t)

pr.print_stats()
for test_index in [0,1,2,499,999] : print(res[test_index])
