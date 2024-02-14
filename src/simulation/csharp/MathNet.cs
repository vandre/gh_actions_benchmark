using System;
using System.Collections.Concurrent;
using System.Threading.Tasks;
using MathNet.Numerics.LinearAlgebra;

public partial class Simulation
{
    double[] sima_par(Vector<double> survival, Vector<double> yield, (Vector<double>, Vector<double>) pol, Vector<double> mort, int n)
    {
        var V = Vector<double>.Build;
        double[] res = new double[yield.Count];
        var rangePartitioner = Partitioner.Create(0, yield.Count);

        Parallel.ForEach(rangePartitioner, (range, state) =>
        {
            for (int k = range.Item1; k < range.Item2; k++)
            {
                var yield_value = yield[k];
                var r = V.Dense(n, 0.0); // accumulated deficiency
                var av = V.Dense(n, 1.0);

                for (int j = 0; j < mort.Count; j++)
                { // number of timepoints
                    var gc = av.PointwiseMultiply(pol.Item1);
                    av -= gc;
                    av = av.Multiply(yield_value);
                    var b = (pol.Item2 - av).Multiply(survival[j] * mort[j]);
                    var e = j + 1.0;
                    r = r.PointwiseAbsoluteMaximum((b - gc) / Math.Pow(yield_value, e));
                }

                res[k] = r.Sum();
            }
        });
        return res;
    }

    double[] sima_m(Vector<double> survival, Vector<double> yield, (Vector<double>, Vector<double>) pol, Vector<double> mort, int n)
    {
        var V = Vector<double>.Build;
        double[] res = new double[yield.Count];
        for (int k = 0; k < yield.Count; k++)
        {
            var yield_value = yield[k];
            var r = V.Dense(n, 0.0); // accumulated deficiency
            var av = V.Dense(n, 1.0);
            for (int j = 0; j < mort.Count; j++)
            { // number of timepoints
                var gc = av.PointwiseMultiply(pol.Item1);
                av -= gc;
                av = av.Multiply(yield_value);
                var b = (pol.Item2 - av).Multiply(survival[j] * mort[j]);
                var e = j + 1.0;
                r = r.PointwiseAbsoluteMaximum((b - gc) / Math.Pow(yield_value, e));
            }
            res[k] = r.Sum();
        }
        return res;
    }
}