using System;
using System.Linq;
using BenchmarkDotNet.Attributes;
using MathNet.Numerics.LinearAlgebra;

public partial class Simulation
{
    public int m = 42  ;
    private const int τ = 120 * 12; // policy duration
    private const int n = 10_000; // number of policies
    private double[] MORT = new double[τ];
    public double[] YIELD = new Double[0];
    private double[] SURVIVAL = new double[n];
    private double[] POL1 = new double[n];
    private double[] POL2 = new double[n];


    [GlobalSetup]
    public void GlobalSetup()
    {
        _ = int.TryParse(Environment.GetEnvironmentVariable("SCENARIOS"), out m);
        if (m == 0) m = 1000;
        Console.WriteLine($"Running {m} scenarios...");
        YIELD = new double[m];
        double[] Σ = LoadData(m);
        double[] ones = new double[MORT.Length];
        double[] p = new double[MORT.Length];
        Array.Fill(MORT, 0.001 / 12.0);
        Array.Fill(ones, 1.0);
        Array.Fill(POL1, 0.02 / 12.0);
        Array.Fill(POL2, 1000.0);
        p.PointwiseSub(ones, MORT);
        SURVIVAL = p.CumProd();
        YIELD.PointwiseAdd(Σ, ones.AsSpan().Slice(0, m).ToArray());
    }

    [Benchmark]
    public string calc_mathnet()
    {
        VectorBuilder<double> V = Vector<double>.Build;
        var res = sima_m(V.Dense(SURVIVAL), V.Dense(YIELD), (V.Dense(POL1), V.Dense(POL2)), V.Dense(MORT), n);
        return $"{res[0]} {res[1]}";
    }

    [Benchmark]
    public string calc_par_mathnet()
    {
        VectorBuilder<double> V = Vector<double>.Build;
        var res = sima_par(V.Dense(SURVIVAL), V.Dense(YIELD), (V.Dense(POL1), V.Dense(POL2)), V.Dense(MORT), n);
        return $"{res[0]} {res[1]}";
    }

    [Benchmark]
    public string calc_span()
    {
        var sm = new double[MORT.Length];
        sm.PointwiseMul(SURVIVAL, MORT);
        ReadOnlySpan<double> yield = YIELD.AsSpan();
        ReadOnlySpan<double> pol1 = POL1.AsSpan();
        ReadOnlySpan<double> pol2 = POL2.AsSpan();
        var res = simasp(sm, yield, pol1, pol2);
        return $"{res[0]} {res[1]}";
    }

    [Benchmark]
    [BenchmarkCategory("simd")]
    public string calc_par_vec()
    {
        var sm = new double[MORT.Length];
        sm.PointwiseMul(SURVIVAL, MORT);
        var res = simapv(sm, YIELD!, POL1, POL2);
        return $"{res[0]} {res[1]}";
    }

    [Benchmark]
    public string calc_par_arr()
    {
        var sm = new double[MORT.Length];
        sm.PointwiseMul(SURVIVAL, MORT);
        var res = simapa(sm, YIELD!, POL1, POL2);
        return $"{res[0]} {res[1]}";
    }
}