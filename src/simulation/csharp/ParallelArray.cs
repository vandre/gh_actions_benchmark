using System;
using System.Threading.Tasks;
public partial class Simulation
{
    double[] simapa(double[] sm, double[] yield, double[] pol1, double[] pol2)
    {
        var res = new double[yield.Length];
        Parallel.For(0, yield.Length, new ParallelOptions() { MaxDegreeOfParallelism = 4 }, k =>
        {
            double yield_value = yield[k];
            double sum = 0;
            Span<double> r = new double[n];
            Span<double> av = new double[n];
            av.Fill(1.0);
            Span<double> gc = new double[n];
            Span<double> bl = new double[n];
            Span<double> b = new double[n];
            for (int j = 0; j < sm.Length; j++)
            { // number of timepoints
                double e = j + 1.0;
                for (int idx = 0; idx < n; idx++)
                {
                    gc[idx] = av[idx] * pol1[idx];
                    av[idx] = av[idx] - gc[idx];
                    av[idx] = av[idx] * yield_value;
                    bl[idx] = pol2[idx] - av[idx];
                    b[idx] = bl[idx] * sm[j];
                    double val = (b[idx] - gc[idx]) / Math.Pow(yield_value, e);
                    r[idx] = Math.Max(val, r[idx]);
                }
            }

            foreach (double val in r) { sum += val; }
            res[k] = sum;
        });
        return res;
    }
}

