use rayon::prelude::*;
use serde::Serialize;
use std::env;
use std::{time::Duration};
use std::fs::File;
use std::io::prelude::*;
use serde_json::{self, to_string_pretty};

#[derive(Serialize)]
struct Stats { pub min: Duration, pub max: Duration, pub avg: Duration }

fn load_data(scen:usize) -> Vec<f64> {
    // Open the file
    let mut file = File::open("../../../data/simulation.json").expect("Unable to open file");
    // Read the file contents into a string
    let mut contents = String::new();
    file.read_to_string(&mut contents).expect("Unable to read file");
    // Deserialize the JSON data into a vector of f64 values
    let data: Vec<f64> = serde_json::from_str(&contents).expect("Unable to parse JSON");
    return data.get(0..scen).unwrap().to_vec();
}

fn sanity_check(res:&[f64]) {
    approx(res[0], 811.4434227662144);
    approx(res[1], 3565.065121402461);
    approx(res[2], 813.5565267078123);
    //approx(res[999], 812.1126188746483);
    println!("Sanity check passed");
}
fn approx(n1:f64,n2:f64) { assert!((n2-n1).abs() < 1e-9);}

#[allow(non_snake_case)]
pub fn sima(res:&mut [f64], survival:&[f64], YIELD:&[f64], POL:&(Vec<f64>,Vec<f64>), MORT:&[f64],n:usize){
    let max_val = |x: f64, y: f64| if x > y { x } else { y };
    let sm:Vec<f64> = survival.iter().zip(MORT.iter()).map(|(a,b)| a * b ).collect();
    let  res_par:Vec<f64> = YIELD.par_iter().map(|&yield_value| { //number of scenarions
        let mut r = vec![0.0;n]; //accumulated deficiency
        let mut av = vec![1.0;n];
        for j in 0..MORT.len() { //number of timepoints
            let gc : Vec<f64> = av.iter().zip(POL.0.iter()).map(|(a,b)| a * b ).collect();
            av = av.iter().zip(gc.iter()).map(|(a,b)|a-b).collect();
            av = av.iter().map(|a|a*yield_value).collect();
            let bl:Vec<f64> = POL.1.iter().zip(av.iter()).map(|(a,b)|a-b).collect();
            let b:Vec<f64> = bl.iter().map(|a| a * sm[j]).collect();
            let e = j as f64 + 1.0;
            for i in 0..r.len() {
                let val = (b[i] - gc[i]) / yield_value.powf(e);
                *r.get_mut(i).unwrap() = max_val(val, r[i]);
            }
        }
        
        r.iter().map(|a|a).sum()
        
    }).collect();
    res.copy_from_slice(&res_par)
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

#[allow(non_snake_case)]
pub fn main(){
    let τ = 120 * 12; // policy duration
    let n = 10_000; // number of policies
    let scen = match env::var("SCENARIOS") { Ok(val) => val, Err(_) => String::new()
    };
    let m = scen.parse::<usize>().unwrap_or(1000);
   
    let MORT=  vec![0.001/12.0; τ];
    let p: Vec<f64> = MORT.iter().map(|&a| { 1.0 - a }).collect();
    let mut survival_prod = 1.0;
    let survival: Vec<f64> = p.iter()
    .map(|&n| { survival_prod *= n; survival_prod }).collect();
    let Σ = load_data(m);
    let POL = (vec![0.02/12.0; n] , vec![1000.0; n]);
    let YIELD: Vec<f64> = Σ.iter().map(|&a| { 1.0 + a }).collect();
    let mut res = vec![0.0; m]; // reserve
    println!("Running {:?} scenarios...", m);
    
    let stats = time_it!("sim_fn:",{
        sima(&mut res,&survival,&YIELD,&POL,&MORT,n);
    }, 5);

    println!("Elapsed time: {:.6?} (avg)", stats.avg);
    sanity_check(&res);
    for test_index in [0,1,2,m-1] { println!("{:?}", res[test_index]); }

    let json_string = to_string_pretty(&stats).unwrap();
    let filename = "stats.json";
    let mut file = File::create(filename).unwrap();
    println!("Saving benchmarks to {:?}", filename);
    let _ = file.write_all(json_string.as_bytes());
}
