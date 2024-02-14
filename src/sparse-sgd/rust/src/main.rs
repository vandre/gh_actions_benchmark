#![allow(non_snake_case)]
use std::{time::Instant};
use nalgebra_sparse::{CsrMatrix};
use nalgebra::{Matrix, VecStorage, Dyn};
use sparse_sgd::*;

fn sgd_V(x:&CsrMatrix<f64>,
    total_losses:&Vec<f64>,
    cross_terms:Matrix<f64, Dyn, Dyn, VecStorage<f64,Dyn,Dyn>>,
    v:Matrix<f64, Dyn, Dyn, VecStorage<f64,Dyn,Dyn>>,
    dv:Matrix<f64, Dyn, Dyn, VecStorage<f64,Dyn,Dyn>>,
)->Matrix<f64, Dyn, Dyn, VecStorage<f64,Dyn,Dyn>>{

    let k:usize=10 ;
    let learning_rate=0.99;
    let m_r=0.1;
    let m_l=0.1;
    
    let mut V = v.clone();
    let mut delta_V = dv.clone();
    let currV = v.clone();

    //X shape is (1475055,101856)
    let xrows:f64 = x.nrows()  as f64;
    let xcols:usize = x.ncols();
    let x_loss =  (&x.multiply_vec(&total_losses))/xrows;
    
    //A difference from Python script is that we don't divide
    // by xrows here, but instead in the loop further down
    let xxl = &x.multiply_csr(&x).multiply_vec(&total_losses).sumnnz(); 

    let xvxl = x_loss.transpose()*cross_terms;

     for f in 0..k {
        for i in 0..xcols {
            V[(i, f)] -= learning_rate * 
                ((xvxl[(i, f)] - xxl[i]/xrows * V[(i, f)]) + 
                m_r * delta_V[(i, f)] + m_l * V[(i, f)]);
        }
    }

    delta_V = currV - v;
    return delta_V
}
fn main() {
    
    let w =  coo_matrix("coo.json");
    let m1 = w.select(0..w.nrows(),0..11) ;
    let m2 = w.select(0..w.nrows(),12..w.ncols()) ;
    let x = hcat(&m1,&m2);
    let y = w.select(0..w.nrows(), 11..12);
    
    let v1 = read_matrix("v1.json", 101856,10);
    
    let cross_terms = x.clone()*v1;
    
    let yrows = y.nrows();
    
    let mut total_losses = vec![0.0; yrows];

    for k in 0..y.row_indices().len() {
        total_losses[k] = y.values()[k];
    }

    let v = read_matrix("v.json", 101856,10);
    let delta_v = read_matrix("dv.json", 101856,10);

    let before = Instant::now();
    sgd_V(&x, &total_losses, cross_terms,v,delta_v);
    let elapsed = before.elapsed();
    println!("Elapsed time: {:.6?} seconds ({:.2?})", elapsed.as_secs_f32(),elapsed);
    //println!("{:?}",result[(0,0)])
    
}    
