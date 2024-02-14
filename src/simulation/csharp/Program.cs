using System;
using System.IO;
using System.Text.Json;
using System.Linq;
using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Running;
using BenchmarkDotNet.Engines;
using System.Diagnostics;
using BenchmarkDotNet.Configs;
using BenchmarkDotNet.Jobs;
using BenchmarkDotNet.Environments;
using Perfolizer.Horology;

[MemoryDiagnoser]
[Orderer(BenchmarkDotNet.Order.SummaryOrderPolicy.FastestToSlowest)]
[RankColumn]
public partial class Simulation
{
    double[] LoadData(int scenarios)
    {
        string path = AppContext.BaseDirectory;
        var file = path.Substring(0, path.IndexOf("src")) + @"data/simulation.json";
        using (StreamReader reader = new StreamReader(file))
        {
            string contents = reader.ReadToEnd();
            var data = JsonSerializer.Deserialize<double[]>(contents);
            return data != null ? data.Skip(0).Take(scenarios).ToArray() : new double[0];
        }
    }
}

public class Program
{
    public static void Main(string[] args)
    {
        BenchmarkSwitcher
        .FromAssembly(typeof(Program).Assembly)
        .Run(args, GetGlobalConfig());

        static IConfig GetGlobalConfig()
        {
            return DefaultConfig.Instance
                .AddJob(new Job("SwiftJob", RunMode.Dry)
                { 
                    Run = 
                    { 
                        IterationCount = 4,
                        WarmupCount = 1, 
                        RunStrategy=RunStrategy.Throughput
                    },
                 
                });
        }
    }
}