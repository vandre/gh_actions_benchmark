using System;
using System.Threading.Tasks;
public partial class Simulation
{
    double[] simapv(double[] sm, double[] yield, double[] pol1, double[] pol2)
    {
        var res = new double[yield.Length];
        Parallel.For(0, yield.Length, new ParallelOptions() { MaxDegreeOfParallelism = 16 }, k =>
        {
            var yield_value = yield[k];
            var r = new double[n];
            var av = new double[n];
            Array.Fill(av, 1.0);
            var gc = new double[n];
            var bl = new double[n];
            var b = new double[n];
            for (int j = 0; j < sm.Length; j++)
            {
                double e = j + 1.0;
                gc.PointwiseMul(av, pol1);
                av.PointwiseSub(av, gc);
                av.MulScalar(av, yield_value);
                bl.PointwiseSub(pol2, av);
                b.MulScalar(bl, sm[j]);
                for (int idx = 0; idx < n; idx++)
                {
                    double val = (b[idx] - gc[idx]) / Math.Pow(yield_value, e);
                    r[idx] = Math.Max(val, r[idx]);
                }
            }
            res[k] = r.Sum();
        });
        return res;
    }
}
