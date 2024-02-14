using System;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

public static class LinearAlgebra
{
    [SkipLocalsInit]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void PointwiseAdd<T>(this T[] result, T[] array1, T[] array2) where T : struct, INumber<T>
    {
        if (array1.Length != array2.Length && array1.Length!=result.Length) throw new ArgumentException();

        int numberSlots = Vector<T>.Count;
        int numVectors = array1.Length / numberSlots;
        int ceiling = numVectors * numberSlots;
        var mem1 = new ReadOnlyMemory<T>(array1);
        var mem2 = new ReadOnlyMemory<T>(array2);
        var memr = new Memory<T>(result);

        ReadOnlySpan<Vector<T>> vsArray1 = MemoryMarshal.Cast<T, Vector<T>>(mem1.Span);
        ReadOnlySpan<Vector<T>> vsArray2 = MemoryMarshal.Cast<T, Vector<T>>(mem2.Span);
        Span<Vector<T>> vsResult = MemoryMarshal.Cast<T, Vector<T>>(memr.Span);

        for (int i = 0; i < numVectors; i++)
        {
            vsResult[i] = Vector.Add(vsArray1[i], vsArray2[i]);
        }
        // Finish operation with any numbers leftover
        for (int i = ceiling; i < array1.Length; i++)
        {
            result[i] = array1[i] + array2[i];
        }
    }
    [SkipLocalsInit]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void PointwiseSub<T>(this T[] result, T[] array1, T[] array2) where T : struct, INumber<T>
    {
        if (array1.Length != array2.Length && array1.Length!=result.Length) throw new ArgumentException();

        int numberSlots = Vector<T>.Count;
        int numVectors = array1.Length / numberSlots;
        int ceiling = numVectors * numberSlots;
        var mem1 = new ReadOnlyMemory<T>(array1);
        var mem2 = new ReadOnlyMemory<T>(array2);
        var memr = new Memory<T>(result);

        ReadOnlySpan<Vector<T>> vsArray1 = MemoryMarshal.Cast<T, Vector<T>>(mem1.Span);
        ReadOnlySpan<Vector<T>> vsArray2 = MemoryMarshal.Cast<T, Vector<T>>(mem2.Span);
        Span<Vector<T>> vsResult = MemoryMarshal.Cast<T, Vector<T>>(memr.Span);

        for (int i = 0; i < numVectors; i++)
        {
            vsResult[i] = Vector.Subtract(vsArray1[i], vsArray2[i]);
        }
        // Finish operation with any numbers leftover
        for (int i = ceiling; i < array1.Length; i++)
        {
            result[i] = array1[i] - array2[i];
        }
    }
    [SkipLocalsInit]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void PointwiseMul<T>(this T[] result, T[] array1, T[] array2) where T : struct, INumber<T>
    {
        if (array1.Length != array2.Length && array1.Length!=result.Length) throw new ArgumentException();

        int numberSlots = Vector<T>.Count;
        int numVectors = array1.Length / numberSlots;
        int ceiling = numVectors * numberSlots;
        var mem1 = new ReadOnlyMemory<T>(array1);
        var mem2 = new ReadOnlyMemory<T>(array2);
        var memr = new Memory<T>(result);

        ReadOnlySpan<Vector<T>> vsArray1 = MemoryMarshal.Cast<T, Vector<T>>(mem1.Span);
        ReadOnlySpan<Vector<T>> vsArray2 = MemoryMarshal.Cast<T, Vector<T>>(mem2.Span);
        Span<Vector<T>> vsResult = MemoryMarshal.Cast<T, Vector<T>>(memr.Span);

        for (int i = 0; i < numVectors; i++)
        {
            vsResult[i] = Vector.Multiply(vsArray1[i], vsArray2[i]);
        }
        // Finish operation with any numbers leftover
        for (int i = ceiling; i < array1.Length; i++)
        {
            result[i] = array1[i] * array2[i];
        }
    }
    [SkipLocalsInit]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void MulScalar<T>(this T[] result, T[] array1, T scalar) where T : struct, INumber<T>
    {
        if (array1.Length!=result.Length) throw new ArgumentException();
        
        int numberSlots = Vector<T>.Count;
        int numVectors = array1.Length / numberSlots;
        int ceiling = numVectors * numberSlots;

        for (int i = 0; i < ceiling; i += numberSlots)
        {
            Vector<T> v1 = new Vector<T>(array1, i);
            Vector.Multiply(v1, scalar).CopyTo(result, i);
        }
        for (int i = ceiling; i < array1.Length; i++)
        {
            result[i] = array1[i] * scalar;
        }
    }
    [SkipLocalsInit]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T Sum<T>(this T[] array) where T : struct, INumber<T>
    {
        int numberSlots = Vector<T>.Count;
        int numVectors = array.Length / numberSlots;
        int ceiling = numVectors * numberSlots;
        T sum;
        Vector<T> vSum = Vector<T>.Zero;
        ReadOnlySpan<Vector<T>> vsArray = MemoryMarshal.Cast<T, Vector<T>>(new Memory<T>(array).Span);
        for (int i = 0; i < numVectors; i++)
        {
            vSum += vsArray[i];
        }
        sum = Vector.Dot(vSum, Vector<T>.One);

        for (int i = ceiling; i < array.Length; i++)
        {
            sum += array[i];
        }
        return sum;
    }
    [SkipLocalsInit]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T[] CumProd<T>(this T[] array) where T : struct, INumber<T>
    {
        T[] result = new T[array.Length];
        T zero = default(T);
        T prod = ++zero;
        ReadOnlySpan<T> span = array.AsSpan();
        for (int i = 0; i < span.Length; i++)
        {
            prod = prod * span[i];
            result[i] = prod;
        }
        return result;
    }
}