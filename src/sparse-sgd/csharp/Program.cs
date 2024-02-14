using BenchmarkDotNet.Configs;
using BenchmarkDotNet.Engines;
using BenchmarkDotNet.Jobs;
using BenchmarkDotNet.Running;

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