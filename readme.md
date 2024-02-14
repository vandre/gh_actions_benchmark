# README

The code repository accompanying the research paper [An actuary's guide to Julia: Use cases and performance benchmarking in insurance](https://www.milliman.com/en/insight/an-actuary-guide-to-julia-use-cases-performance-benchmarking-insurance).

---
## Environment settings

- Julia version 1.10.0
- Python version 3.12.1 with pip version 24.0
- C# with .net sdk version 7.0
- Rust version 1.75.0
- CMake version 3.8 with Boost version 1.82.0

---
## Repository structure

- Sample implementations of data processing for Julia and Python can be found in `ML` and `munging` in the `src` folder. The authors have included them here for easier syntax comparisons between the two languages. Benchmarking results were not included in the research paper. To run the code segments, follow the instructions specified in the `README.md` files.
- Sample implementations of the three use cases can be found in `similarity`, `simulation` and `sparse-sgd` in the `src` folder. To run the programs, follow the instructions specified in the `README.md` files. These samples should be deemed for educational purposes only.
- The `data` folder contains sample data for various use cases. Except for `housing.csv` which is from one of the open-sourced datasets [Boston house price](https://lib.stat.cmu.edu/datasets/boston), all other datasets are synthetic and should be deemed for educational purposes only.

**NOTE:** Prior to running any of the programs the user should unzip the files in the `data` directory. Shell scripts for Windows (`unzip-all-data-windows.bat`) and Linux (`unzip-all-data-linux.sh`) are included in the `data` directory and can be run once to facilitate the required unzipping. (The Linux script may be "sourced" into the shell, as in `source unzip-all-data-linux.sh`, or assigned the executable bit with `chmod +x unzip-all-data-linux.sh` and then run with `./unzip-all-data-linux.sh`.) Once unzipped, the original archive files remain, but subsequent invocation of the appropriate unzip script will not overwrite the extracted files. To restore the original zipped content, the unzipped data files should first be removed, after which the appropriate unzip script can be run again to extract the archived content.

---
## GitHub Actions scripts

- .yml files for GitHub Actions to run the three use cases are available to get benchmarking results on different languages. The scripts are most up-to-date as of Feb 2024. The latest `julia-action` still generates some warning messages which should not affect the benchmarking results.
- Each script loops through each language, installing required libraries and running corresponding programs in different languages, and finally a reporting script collecting benchmarking results.
