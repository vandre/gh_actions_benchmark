use rand::Rng;
use rayon::prelude::*;
use serde::Serialize;
use std::{time::Duration};
use std::fs::File;
use std::io::prelude::*;
use serde_json::{self, to_string_pretty};

#[derive(Serialize)]
struct Stats { pub min: Duration, pub max: Duration, pub avg: Duration }


pub fn and_count(a: &[i32], b: &[i32]) -> u32 {
    let vec: Vec<i32> = a.iter().zip(b.iter()).map(|(&a, &b)| a & b).collect();
    let count = vec.iter().map(|&byte| byte.count_ones()).sum();
    return count;
}

pub fn and_count_par(a: &[i32], b: &[i32]) -> u32 {
    let vec: Vec<i32> = a.par_iter().zip(b.par_iter()).map(|(&a, &b)| a & b).collect();
    let count = vec.par_iter().map(|&byte| byte.count_ones()).sum();
    return count;
}

pub fn rand_vec(bits: usize) -> Vec<i32> {
    let vec_size = ((bits as f32 / 32f32).ceil()) as usize;
    let mut rng = rand::thread_rng();
    let mut vec = vec![0; vec_size];
    rng.fill(&mut vec[..]);
    // Check if last element of the vector has 'spare bits',
    // If this is the case we want to zero-out those bits to have an accurate bit count
    if bits % 32 != 0 {
        let n: i32 = (32 - (bits % 32)).try_into().unwrap(); // number of bits to zero-out
        let mask = (1 << n) - 1; // construct the mask
        let last_index = vec.len() - 1;
        if let Some(last) = vec.get_mut(last_index) {
            *last &= !mask; // bitwise-AND with the complement of the mask
        }
    }
    return vec;
}

macro_rules! time_it {
    ($context:literal, $($stmt:stmt);+ $(;)? $(,$iter:expr)?) => {{
        let mut timer = std::time::Instant::now();
        let mut runs = Vec::new();
        let mut it = 1;
        $ ( it = if $iter > it  { $iter } else{ it }; )? 
        for _ in 0..it {
            $($stmt);+
            runs.push(timer.elapsed());
            timer = std::time::Instant::now();
        }
        let min = runs.iter().min().unwrap();
        let max = runs.iter().max().unwrap();
        let avg = runs.iter().sum::<Duration>() as Duration/runs.len() as u32;
        println!("{} min: {:?} max: {:?} avg: {:?}", $context, min, max, avg);
        Stats {min:*min,max:*max,avg:avg }
    }};
}

pub fn main() {
    let bits = 100_000_000;
    let x = rand_vec(bits);
    let y = rand_vec(bits);
    println!("\n@Benchmark");
    println!("Processed {:?} bits", bits);
    
    let stats = time_it!("similarity_fn:",{
        and_count_par(&x,&y);
    }, 500);
    
    println!("Elapsed time: {:.6?} (avg)", stats.avg);
    let json_string = to_string_pretty(&stats).unwrap();
    let filename = "stats.json";
    let mut file = File::create(filename).unwrap();
    println!("Saving benchmarks to {:?}", filename);
    let _ = file.write_all(json_string.as_bytes());
}
