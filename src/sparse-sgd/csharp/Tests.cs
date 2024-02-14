using System.Diagnostics;
using System.Linq;

public static class SparseTest
{
    public static void SelectTest()
    {
        double[] v = { 1.0, 3.0, 2.0, 4.0 };
        int[] i = { 0, 0, 1, 2 };
        int[] j = { 0, 2, 0, 2 };
        int[] s ={3,3};
        var d = new CooPickle(){row= i, col=j, data=v, shape=s};
        var a = d.Select(0..2, 0..2);
        var b = d.Select(1..3, 1..3);

        Debug.Assert(2 == a.row.Length);
        Debug.Assert(2 == a.col.Length);
        Debug.Assert(Enumerable.SequenceEqual(new double[] { 1.0, 2.0 }, a.data));
        Debug.Assert(Enumerable.SequenceEqual(new int[] { 2,2 },a.shape));

        Debug.Assert(1 == b.row.Length);
        Debug.Assert(1 == b.col.Length);
        Debug.Assert(Enumerable.SequenceEqual(new double[] { 4.0 }, b.data));
        Debug.Assert(Enumerable.SequenceEqual(new int[] { 2,2 },b.shape));
    }

    public static void HCatTest()
    {
        double[] v = { 1.0, 3.0, 2.0, 4.0 };
        int[] i = { 0, 0, 1, 2 };
        int[] j = { 0, 2, 0, 2 };
        int[] s ={3,3};
        var d = new CooPickle(){row= i, col=j, data=v, shape=s};
        var m = SparseUtil.HCat(d, d);

        Debug.Assert(3 == m.nrows());
        Debug.Assert(6 == m.ncols());
        Debug.Assert(Enumerable.SequenceEqual(new double[] { 1.0, 3.0, 1.0, 3.0, 2.0, 2.0, 4.0, 4.0 }, m.data));

    }
}