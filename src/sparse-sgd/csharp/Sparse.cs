using System;
using System.Collections.Generic;
using System.Linq;

public static class SparseUtil
{
    public static CooPickle Select(this CooPickle coo, Range row, Range col)
    {
        var row_idx = coo.row;
        var col_idx = coo.col;
        var val = coo.data;

        int nnz = 0;
        for (int k = 0; k < row_idx.Length; k++)
        {
            if (row_idx[k] >= row.Start.Value && row_idx[k] < row.End.Value && col_idx[k] >= col.Start.Value && col_idx[k] < col.End.Value)
            {
                nnz += 1;
            }
        }

        // Initialize arrays to hold the sliced COO matrix data
        int[] i_sliced = new int[nnz];
        int[] j_sliced = new int[nnz];
        double[] v_sliced = new double[nnz];
        var sliced = new CooPickle();

        // Populate the sliced COO matrix arrays
        int idx = 0;
        for (int k = 0; k < row_idx.Length; k++)
        {
            if (row_idx[k] >= row.Start.Value && row_idx[k] < row.End.Value && col_idx[k] >= col.Start.Value && col_idx[k] < col.End.Value)
            {
                i_sliced[idx] = row_idx[k] - row.Start.Value;
                j_sliced[idx] = col_idx[k] - col.Start.Value;
                v_sliced[idx] = val[k];
                idx += 1;
            }
        }

        sliced.row = i_sliced;
        sliced.col = j_sliced;
        sliced.data = v_sliced;
        sliced.shape = new int[]{row.End.Value-row.Start.Value, col.End.Value-col.Start.Value};
        return sliced;

    }
    public static CooPickle HCat(CooPickle m1, CooPickle m2)
    {
        int num_rows1 = m1.nrows();
        int num_rows2 = m2.nrows();

        int num_cols1 = m1.ncols();
        int num_cols2 = m2.ncols();

        if (num_rows1 != num_rows2)
        {
            throw new ArgumentException("Number of rows must be equal");
        }

        int[] i1 = m1.row;
        int[] j1 = m1.col;
        double[] v1 = m1.data;

        int[] i2 = m2.row;
        int[] j2 = m2.col;
        double[] v2 = m2.data;

        int num_nonzeros1 = i1.Length;
        int num_nonzeros2 = i2.Length;

        int[] i = new int[num_nonzeros1 + num_nonzeros2];
        int[] j = new int[num_nonzeros1 + num_nonzeros2];
        double[] v = new double[num_nonzeros1 + num_nonzeros2];

        for (int k = 0; k < num_nonzeros1; k++)
        {
            i[k] = i1[k];
            j[k] = j1[k];
            v[k] = v1[k];
        }

        for (int k = 0; k < num_nonzeros2; k++)
        {
            i[k + num_nonzeros1] = i2[k];
            j[k + num_nonzeros1] = j2[k] + num_cols1;
            v[k + num_nonzeros1] = v2[k];
        }

        var triplet = new (int, (int, double))[i.Length];
        for (int k = 0; k < i.Length; k++)
        {
            triplet[k] = (i[k], (j[k], v[k]));
        }
        Array.Sort(triplet);
        
        var ti = new int[triplet.Length];
        var tj = new int[triplet.Length];
        var tv = new double[triplet.Length];
        
        for (int k = 0; k < i.Length; k++)
        {
            ti[k] = triplet[k].Item1;
            tj[k] = triplet[k].Item2.Item1;
            tv[k] = triplet[k].Item2.Item2;
        }

        int[] s = new int[] { num_rows1, num_cols1 + num_cols2 };
        return new CooPickle(){row=ti, col=tj, data=tv, shape=s};
    }
    public static CooPickle MultiplyByColumnThenDivide(this CooPickle m, CooPickle other, double scalar)
    {
        if (other.shape[1] != 1) { throw new InvalidOperationException("Must be a vector column"); }
        var m_rowindices = m.row;
        var m_colindices = m.col;
        var m_values = m.data;

        var v_rowindices = other.row;
        var v_values = other.data;

        var intersect = m_rowindices.Intersect(v_rowindices).ToHashSet();
        var cooHash = new Dictionary<(int, int), double>();
        for (int i = 0; i < m_rowindices.Length; i++)
        {
            if (intersect.Contains(m_rowindices[i]))
            {
                cooHash.Add((m_rowindices[i], m_colindices[i]), m_values[i]);
            }
        }

        var vHash = new Dictionary<int, double>();
        for (int i = 0; i < v_rowindices.Length; i++)
        {
            if (intersect.Contains(v_rowindices[i]))
            {
                vHash.Add(v_rowindices[i], v_values[i]);
            }
        }

        var r_rowindices = new List<int>();
        var r_colindices = new List<int>();

        foreach (var key in cooHash.Keys)
        {

            if (vHash.Keys.Contains(key.Item1))
            {
                cooHash[key] *= vHash[key.Item1] / scalar;
            }

            r_rowindices.Add(key.Item1);
            r_colindices.Add(key.Item2);
        }

        var res = new CooPickle()
        {
            shape = m.shape,
            row = r_rowindices.ToArray(),
            col = r_colindices.ToArray(),
            data = cooHash.Values.ToArray()
        };

        return res;
    }
    public static CooPickle Squared(this CooPickle m)
    {

        var m_rowindices = m.row;
        var m_colindices = m.col;
        var m_values = m.data;

        for (int i = 0; i < m_values.Length; i++)
        {
            m_values[i] *= m_values[i];
        }

        var res = new CooPickle()
        {
            shape = m.shape,
            row = m_rowindices,
            col = m_colindices,
            data = m_values
        };

        return res;
    }
    public static double[] FoldByAxis0ThenDivide(this CooPickle m, double scalar)
    {

        var dict = new Dictionary<int, double>();

        for (int i = 0; i < m.data.Length; i++)
        {
            double value = m.data[i];
            int key = m.col[i];

            if (dict.ContainsKey(key))
            {
                dict[key] += value;
            }
            else
            {
                dict[key] = value;
            }
        }
        var keys = dict.Keys.ToArray();
        Array.Sort(keys);
        var result = new double[keys.Length];
        for (int i = 0; i < keys.Length; i++)
        {
            result[i] = dict[keys[i]] / scalar;
        }
        return result;
    }

}

public struct CooPickle
{
    /// <summary>
    /// The row indices of the matrix entries
    /// </summary>
    public int[] row { get; set; }
    /// <summary>
    /// The column indices of the matrix entries
    /// </summary>
    public int[] col { get; set; }
    /// <summary>
    /// The entries of the matrix, in any order
    /// </summary>
    public double[] data { get; set; }
    /// <summary>
    /// The MxN shape of the matrix
    /// </summary>
    public int[] shape { get; set; }
    public int nrows() { return shape[0]; }
    public int ncols() { return shape[1]; }
}