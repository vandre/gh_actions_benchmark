import os
import re
import json
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns



def read_json(filename):
    with open(filename, "r") as f:
        obj = json.load(f)
        f.close()
        return obj


dfName = os.getenv('DF')
dfUnit = ('ms' if os.getenv('PLOT_SECS') is None
          else 's')  # Absent for ms
dfFactorSecs = (1000 if dfUnit == 'ms'
                else 1)  # Python bench => secs
dfFactorNanos = (1e-6 if dfUnit == 'ms'
                 else 1e-9)  # Rust,CSharp,Julia => ns
Language = []
Time = []
error = {}

if (os.path.exists("bench.py.json")):
    py = read_json("bench.py.json")
    pmin = py["benchmarks"][0]["stats"]["min"] * dfFactorSecs
    pmax = py["benchmarks"][0]["stats"]["max"] * dfFactorSecs
    pavg = py["benchmarks"][0]["stats"]["mean"] * dfFactorSecs
    Language.append("Python")
    Time.append(pavg)
    error[pavg] = {'max': pmax, 'min': pmin}

if (os.path.exists("bench.cy.json")):
    with open("bench.cy.json", "r") as file: content = file.readline().strip()
    cy = json.loads(content)
    cymin = cy["min_s"] * dfFactorSecs
    cymax = cy["max_s"] * dfFactorSecs
    cyavg = cy["mean_s"] * dfFactorSecs
    Language.append("Cython")
    Time.append(cyavg)
    error[cyavg] = {'max': cymax, 'min': cymin}

if (os.path.exists("bench.jl.json")):
    jl = read_json("bench.jl.json")
    jmin = jl["min_ns"]*dfFactorNanos
    jmax = jl["max_ns"]*dfFactorNanos
    javg = jl["mean_ns"]*dfFactorNanos
    Language.append("Julia")
    Time.append(javg)
    error[javg] = {'max': jmax, 'min': jmin}

if (os.path.exists("bench.cs.json")):
    cs = read_json("bench.cs.json")
    csmin = cs["Benchmarks"][0]["Statistics"]["Min"] * dfFactorNanos
    csmax = cs["Benchmarks"][0]["Statistics"]["Max"] * dfFactorNanos
    csavg = cs["Benchmarks"][0]["Statistics"]["Mean"] * dfFactorNanos
    Language.append("C#")
    Time.append(csavg)
    error[csavg] = {'max': csmax, 'min': csmin}

if (os.path.exists("bench.cpp.json")):
    cpp = read_json("bench.cpp.json")
    cppmin = cpp["min_seconds"]*dfFactorSecs
    cppmax = cpp["max_seconds"]*dfFactorSecs
    cppavg = cpp["mean_seconds"]*dfFactorSecs
    Language.append("C++")
    Time.append(cppavg)
    error[cppavg] = {'max': cppmax, 'min': cppmin}

if (os.path.exists("bench.rs.json")):
    rs = read_json("bench.rs.json")
    rsmin = rs["min"]["nanos"] * dfFactorNanos \
        + rs["min"]["secs"] * dfFactorSecs
    rsmax = rs["max"]["nanos"] * dfFactorNanos \
        + rs["max"]["secs"] * dfFactorSecs
    rsavg = rs["avg"]["nanos"] * dfFactorNanos \
        + rs["avg"]["secs"] * dfFactorSecs
    Language.append("Rust")
    Time.append(rsavg)
    error[rsavg] = {'max': rsmax, 'min': rsmin}

df = pd.DataFrame({dfName: Language, f"Time ({dfUnit})": Time})

# plot the figure
fig, ax = plt.subplots(figsize=(8, 6))
sns.despine(bottom=True, left=True)
sns.barplot(x=dfName, y=f"Time ({dfUnit})", data=df, ax=ax)

# add the lines for the errors
for p in ax.patches:
    x = p.get_x()  # get the bottom left x corner of the bar
    w = p.get_width()  # get width of bar
    h = p.get_height()  # get height of bar
    min_y = error[h]['min']  # use h to get min from dict z
    max_y = error[h]['max']  # use h to get max from dict z
    # draw a vertical line
    plt.vlines(x+w/2, min_y, max_y, color='.75', linestyles='dashed')

for i in ax.containers:
    ax.bar_label(i, padding=5)

plt.savefig("report.png")

md = "|Language|Mean|Min|Max|\n|---|---:|---:|---:|\n"

if (os.path.exists("bench.py.json")):
    md += f"|Python|{pavg:.2f}{dfUnit}" \
          f"|{pmin:.2f}{dfUnit}|{pmax:.2f}{dfUnit}|\n"

if (os.path.exists("bench.cy.json")):
    md += f"|Cython|{cyavg:.2f}{dfUnit}" \
          f"|{cymin:.2f}{dfUnit}|{cymax:.2f}{dfUnit}|\n"

if (os.path.exists("bench.rs.json")):
    md += f"|Rust|{rsavg:.2f}{dfUnit}" \
          f"|{rsmin:.2f}{dfUnit}|{rsmax:.2f}{dfUnit}|\n"

if (os.path.exists("bench.jl.json")):
    md += f"|Julia|{javg:.2f}{dfUnit}" \
          f"|{jmin:.2f}{dfUnit}|{jmax:.2f}{dfUnit}|\n"

if (os.path.exists("bench.cs.json")):
    md += f"|C#|{csavg:.2f}{dfUnit}" \
          f"|{csmin:.2f}{dfUnit}|{csmax:.2f}{dfUnit}|\n"

if (os.path.exists("bench.cpp.json")):
    md += f"|C++|{cppavg:.2f}{dfUnit}" \
          f"|{cppmin:.2f}{dfUnit}|{cppmax:.2f}{dfUnit}|\n"

mdfile = open("report.md", "w")
mdfile.write(md)
mdfile.close()
