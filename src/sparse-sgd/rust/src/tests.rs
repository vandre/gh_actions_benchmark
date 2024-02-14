#[cfg(test)]
mod tests {
    use nalgebra_sparse::CooMatrix;
    use sparse_sgd::*;
    
    #[test]
    fn select_test() {
        let values =  vec![1.0, 3.0, 2.0, 4.0];
        let indices = vec![0, 0, 1, 2];
        let indptr = vec![0, 2, 0, 2] ;
        let d = CooMatrix::try_from_triplets(3,3, indices, indptr, values).unwrap();
        let a = d.select(0..2,0..2);
        let b = d.select(1..3,1..3);
        
        assert_eq!(a.nrows(),2);
        assert_eq!(a.ncols(),2);
        assert_eq!(a.values(),vec![1.0,2.0]);

        assert_eq!(b.nrows(),2);
        assert_eq!(b.ncols(),2);
        assert_eq!(b.values(),vec![4.0]);

                
    }

    #[test]
    fn hcat_test() {
        let v =  vec![1.0, 3.0, 2.0, 4.0];
        let i = vec![0, 0, 1, 2];
        let j = vec![0, 2, 0, 2] ;
        let d = CooMatrix::try_from_triplets(3,3, i, j, v).unwrap();
        
        let m = hcat(&d, &d);
        assert_eq!(m.nrows(),3);
        assert_eq!(m.ncols(),6);
        assert_eq!(m.values(),vec![1.0, 3.0, 1.0, 3.0, 2.0, 2.0, 4.0, 4.0]);
    }
}