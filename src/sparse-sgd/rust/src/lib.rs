use std::ops::Range;
use std::{fs::File};
use std::io::prelude::*;
use nalgebra::{DMatrix, Matrix, VecStorage, Dyn};
use serde_json::from_str;
use serde::{Deserialize, Serialize};
use nalgebra_sparse::{CooMatrix, CsrMatrix};

#[derive(Serialize, Deserialize)]
pub struct Coo {
    pub indices: Vec<i32>,
    pub indptr: Vec<i32>,
    pub values: Vec<f64>,
    pub shape: (i32,i32)
}

#[derive(Debug,Serialize, Deserialize)]
struct CooPickle {
    pub indices: Vec<usize>,
    pub indptr: Vec<usize>,
    pub values: Vec<f64>,
    pub shape: (usize,usize)
}

pub fn coo_matrix(filename:&str) -> CooMatrix<f64> {
    // Open the file
    let path = format!("../../../data/{filename}");
    let mut file = File::open(path).expect("Unable to open file");
    let mut contents = String::new();
    file.read_to_string(&mut contents).expect("Unable to read file");
    let json: CooPickle = from_str(&contents).expect("Unable to parse JSON");
    return CooMatrix::try_from_triplets(json.shape.0 as usize, json.shape.1 as usize, json.indices, json.indptr, json.values).unwrap();
}


pub trait Select<T> {
    fn select(&self, row:Range<usize>, col:Range<usize>) -> CooMatrix<T>;
}

pub trait Multiply<T> {
    fn multiply_vec(&self, vec:&Vec<T>) -> CsrMatrix<T>;
    fn multiply_csr(&self, csr:&CsrMatrix<T>) -> CsrMatrix<T>;
}

pub trait SumElements {
    fn sumnnz(&self) -> Vec<f64>;
}
impl SumElements for CsrMatrix<f64>  {
    fn sumnnz(&self) -> Vec<f64> {
        let mut sum_cols:Vec<f64>  = vec![0.0;self.ncols()];
        //Axis 0 is iterate through columns
        for idx in 0..self.ncols(){
            sum_cols[idx] = self.get_row(idx).iter().flat_map(|a|a.values()).sum();
        }

       return sum_cols;
    }
}

impl<T> Select<T> for CooMatrix<T> where T: Default + Copy + Clone {
    fn select(&self, row:Range<usize>, col:Range<usize>) -> CooMatrix<T> {
        let row_idx = self.row_indices();
        let col_idx = self.col_indices();
        let val = self.values();

        let mut nnz = 0;
        for k in 0..row_idx.len() {
            if row_idx[k] >= row.start && row_idx[k] < row.end && col_idx[k] >= col.start && col_idx[k] < col.end {
                nnz += 1;
            }
        }

        // Initialize vectors to hold the sliced COO matrix data
        let mut i_sliced = vec![0; nnz];
        let mut j_sliced = vec![0; nnz];
        let mut v_sliced = vec![T::default();nnz];

        // Populate the sliced COO matrix vectors
        let mut idx = 0;
        for k in 0..row_idx.len() {
            if row_idx[k] >= row.start && row_idx[k] < row.end && col_idx[k] >= col.start && col_idx[k] < col.end {
                i_sliced[idx] = row_idx[k] - row.start;
                j_sliced[idx] = col_idx[k] - col.start;
                v_sliced[idx] = val[k];
                idx += 1;
            }
        }
        return CooMatrix::try_from_triplets(row.len(), col.len(), i_sliced, j_sliced, v_sliced).unwrap();

    }
}

impl<T> Multiply<T> for CsrMatrix<T> where T: Default + Copy + Clone + std::ops::Mul<Output = T> {
    fn multiply_vec(&self, vec:&Vec<T>) -> CsrMatrix<T>{
        let v = self.values();

        if self.nrows() != vec.len() {
            panic!("Error: input vector length {:?} is not compatible 
            with the sparse matrix size ({:?},{:?}) ", vec.len(),self.nrows(),self.ncols());
        }

        let mut values = vec![T::default();v.len()];
        for k in 0..vec.len() {
            values[k] = vec[k] * v[k];
        }

        // Initialize vectors to hold the CSR data
        let num_rows = self.nrows();
        let num_cols = self.ncols();
        let row_offsets = self.row_offsets().to_vec();
        let col_indices = self.col_indices().to_vec();

      
        return CsrMatrix::try_from_csr_data(num_rows,num_cols,row_offsets,col_indices,values).unwrap();

    }
    
    fn multiply_csr(&self, csr:&CsrMatrix<T>) -> CsrMatrix<T>{
        let v1 = self.values();
        let v2 = csr.values();

        if self.nrows() != csr.nrows() || self.ncols()!=csr.ncols() {
            panic!("Error: input matrix ({:?},{:?}) is not compatible 
            with the sparse matrix size ({:?},{:?}) ",csr.nrows(),csr.ncols(),self.nrows(),self.ncols());
        }

        let mut values = vec![T::default();v1.len()];
        for k in 0..v1.len() {
            values[k] = v1[k] * v2[k];
        }

        // Initialize vectors to hold the CSR data
        let num_rows = self.nrows();
        let num_cols = self.ncols();
        let row_offsets = self.row_offsets().to_vec();
        let col_indices = self.col_indices().to_vec();

        return CsrMatrix::try_from_csr_data(num_rows,num_cols,row_offsets,col_indices,values).unwrap();

    }
}
pub fn hcat(m1:&CooMatrix<f64>, m2:&CooMatrix<f64>) -> CsrMatrix<f64> {
    let num_rows1 = m1.nrows();
    let num_rows2 = m2.nrows();
    
    let num_cols1 = m1.ncols();
    let num_cols2 = m2.ncols();

    assert!(num_rows1 == num_rows2 );

    let i1= m1.row_indices();
    let j1= m1.col_indices();
    let v1= m1.values();

    let i2= m2.row_indices();
    let j2= m2.col_indices();
    let v2= m2.values();
    
    let num_nonzeros1 = i1.len();
    let num_nonzeros2 = i2.len();

    let mut i = vec![0; num_nonzeros1 + num_nonzeros2];
    let mut j = vec![0; num_nonzeros1 + num_nonzeros2];
    let mut v = vec![0.0; num_nonzeros1 + num_nonzeros2];

    
    for k in 0..num_nonzeros1 {
        i[k] = i1[k];
        j[k] = j1[k];
        v[k] = v1[k];
    }

    for k in 0..num_nonzeros2 {
        i[k + num_nonzeros1] = i2[k];
        j[k + num_nonzeros1] = j2[k] + num_cols1 ;
        v[k + num_nonzeros1] = v2[k];
    }
    let shape=(num_rows1,num_cols1 + num_cols2);
    //return Coo { indices:i, indptr:j, values:v, shape: shape };
    let coo = CooMatrix::try_from_triplets(shape.0, shape.1, i.to_vec(), j.to_vec(), v.to_vec()).unwrap();
    return CsrMatrix::from(&coo);
}

fn reshape(arr1: Vec<f64>, rows: usize, cols: usize) -> Vec<Vec<f64>> {
    if arr1.len() != rows * cols {
        panic!("Error: input vector length is not compatible with the requested matrix size");
    }
    
    arr1.chunks(cols)
        .map(|chunk| chunk.to_vec())
        .collect()
}

fn to_matrix(filename:&str,rows:usize, cols: usize) -> Vec<Vec<f64>> {
    // Open the file
    let path = format!("../../../data/{filename}");
    let mut file = File::open(path).expect("Unable to open file");
    // Read the file contents into a string
    let mut contents = String::new();
    file.read_to_string(&mut contents).expect("Unable to read file");
    // Deserialize the JSON data into a vector of f64 values
    let flat: Vec<f64> = serde_json::from_str(&contents).expect("Unable to parse JSON");
    return  reshape(flat,rows,cols);
}

pub fn read_matrix(filename:&str,rows:usize, cols: usize) -> Matrix<f64, Dyn, Dyn, VecStorage<f64,Dyn,Dyn>> {
    let vec = to_matrix(filename, rows, cols);
    let matrix = DMatrix::from_fn(rows, cols, |i, j| vec[i][j]);
    matrix.into()
}