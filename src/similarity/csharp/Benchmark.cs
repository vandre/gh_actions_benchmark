using System;
using System.Collections.Concurrent;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using BenchmarkDotNet.Attributes;

[MemoryDiagnoser]
[Orderer(BenchmarkDotNet.Order.SummaryOrderPolicy.FastestToSlowest)]
[RankColumn]
//[SimpleJob(RunStrategy.Throughput, launchCount: 1, warmupCount: 5, iterationCount: 10, id: "FastAndDirtyJob")]
public partial class Similarity
{
    
    const int bits = 100_000_000;
    const int n = bits / 32;
    int[] x = new int[n];
    int[] y = new int[n];
    Random random = new Random();

    [GlobalSetup]
    public void GlobalSetup()
    {
        for (int i = 0; i < n; i++)
        {
            x[i] = random.Next();
            y[i] = random.Next();
        }
    }

    [Benchmark]
    public int cnt_For()
    {
        Int32 countOnes = 0;
        for (int i = 0; i < n; i++)
        {
            countOnes += Int32.PopCount(x[i] & y[i]);
        }
        return countOnes;
    }

    
    [Benchmark]
    public int cnt_ForSpan()
    {
        Int32 countOnes = 0;
        var xs = x.AsSpan();
        var ys = y.AsSpan();
        for (int i = 0; i < n; i++)
        {
            countOnes += Int32.PopCount(xs[i] & ys[i]);
        }
        return countOnes;
    }

    [Benchmark]
    public int cnt_ForEach()
    {
        Int32 countOnes = 0;
        foreach (var (a, b) in x.Zip(y))
        {
            countOnes += Int32.PopCount(a & b);
        }
        return countOnes;
    }

    [Benchmark]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public int cnt_ParForEach()
    {
        int countOnes = 0;
        Partitioner<Tuple<int, int>> partitioner = Partitioner.Create(0, n);
        Parallel.ForEach(partitioner, range =>
        {
            int localCountOnes = 0;
            var xs = x.AsSpan();
            var ys = y.AsSpan();
            for (int i = range.Item1; i < range.Item2; i++)
            {
                localCountOnes += int.PopCount(xs[i] & ys[i]);
            }

            Interlocked.Add(ref countOnes, localCountOnes);
        });
        return countOnes;
    }
        
    [Benchmark]
    public async Task<int> cnt_ParTasksAsync()
    {
        Int32 countOnes = 0;
        Int32[] split = new int[Environment.ProcessorCount];
        Task[] tasks = new Task[split.Length];
        Int32 take = (int)Math.Round((double)x.Length / tasks.Length, MidpointRounding.AwayFromZero);

        for (var i = 0; i < tasks.Length; i++)
        {
            var taskNumber = i;
            tasks[i] = Task.Run(() =>
            {
                var localCount = 0;
                for (var index = taskNumber * take; index < (taskNumber + 1) * take && index < x.Length; index++)
                {
                    localCount += int.PopCount(x[index] & y[index]);
                }

                split[taskNumber] = localCount;
            });
        }
        await Task.WhenAll(tasks);
        countOnes = split.Sum();
        return countOnes;
    }


}