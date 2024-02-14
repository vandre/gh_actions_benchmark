using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Engines;
using MathNet.Numerics.LinearAlgebra;
using System.Linq;

[MemoryDiagnoser]
[Orderer(BenchmarkDotNet.Order.SummaryOrderPolicy.FastestToSlowest)]
[RankColumn]
//[SimpleJob(RunStrategy.Throughput, launchCount: 1, warmupCount: 5, iterationCount: 10, id: "FastAndDirtyJob")]
public partial class Sparse
{
    public Matrix<double>? X { get; set; }
    public Vector<double>? Total_Losses { get; set; }
    public Matrix<double>? Cross_Terms { get; set; }
    public Matrix<double>? V { get; set; }
    public Matrix<double>? Delta_V { get; set; }
    public CooPickle x { get; set; }
    public CooPickle y { get; set; }

    [GlobalSetup]
    public void GlobalSetup()
    {
        
        var w = LoadData();
        var m1 = w.Select(0..w.nrows(), 0..11);
        var m2 = w.Select(0..w.nrows(), 12..w.ncols());
        x = SparseUtil.HCat(m1, m2);
        y = w.Select(0..w.nrows(), 11..12);
        X = ToMatrix(x);
        V = LoadMatrixFromArrayFile("v1.json", 101856, 10);
        Cross_Terms = X * V;
        V = LoadMatrixFromArrayFile("v.json", 101856, 10);
        Delta_V = LoadMatrixFromArrayFile("dv.json", 101856, 10);
     }

    [Benchmark]
    public void sgd()
    {
        sgd_V(x, y, Cross_Terms, V!, Delta_V!);
    }
    public Matrix<double>? sgd_V(CooPickle x, CooPickle total_losses, Matrix<double>? cross_terms, Matrix<double> v, Matrix<double> dv)
    {
        var k = 10;
        var learning_rate = 0.99;
        var m_r = 0.1;
        var m_l = 0.1;
        var currV = v.Clone();
        var x_loss = x.MultiplyByColumnThenDivide(total_losses, x.shape[0]);
        var xxl_arr = x.Squared()
                    .MultiplyByColumnThenDivide(total_losses,1.0)
                    .FoldByAxis0ThenDivide(x.shape[0]);
        var xxl = Matrix<double>.Build.DenseOfRowMajor(1, xxl_arr.Length, xxl_arr);

        var xvxl = ToMatrix(x_loss).Transpose() * (cross_terms);
        for (int f = 0; f < k; f++)
        {
            for (int i = 0; i < x.shape[1]; i++)
            {
                v[i, f] -= learning_rate * 
                    ((xvxl[i, f] - xxl[0, i] * v[i, f]) + 
                    m_r * dv[i, f] + m_l * v[i, f]);
            }
        }
        dv = currV - v;
        return dv;
    }

    CooPickle LoadData()
    {
        string path = AppContext.BaseDirectory;
        var file = path.Substring(0, path.IndexOf("src")) + @"data/coo.json";
        using (StreamReader reader = new StreamReader(file))
        {
            string contents = reader.ReadToEnd();
            var data = JsonSerializer.Deserialize<CooPickle>(contents);
            return data;
        }
    }

    Matrix<double> LoadMatrixFromArrayFile(string filename, int rows, int cols) {
        string path = AppContext.BaseDirectory;
        var file = path.Substring(0, path.IndexOf("src")) + $"data/{filename}";
        using (StreamReader reader = new StreamReader(file))
        {
            string contents = reader.ReadToEnd();
            var flatArray = JsonSerializer.Deserialize<double[]>(contents);
            var matrix = Matrix<double>.Build.DenseOfRowMajor(rows, cols, flatArray);
            return matrix;
        }
    }

    static Matrix<double> ToMatrix(CooPickle m)
    {
        var M = Matrix<double>.Build;
        var mtx = M.SparseFromCoordinateFormat(m.shape[0], m.shape[1],
            m.data.Length, m.row, m.col, m.data);
        return mtx;
    }
}