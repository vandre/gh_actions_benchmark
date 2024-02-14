# README

## Building
The similarity code requires [boost](https://www.boost.org/) for linking and header-inclusion, and [CMake](https://cmake.org/)
(along with Visual Studio or gcc as appropriate) for building.

CMake should be installed and in the system path. To create build files for the sparse-sgd project, `cmake` should be run
with its working directory set to the location of this README file.  On Windows, cmake will create a Visual Studio solutions
file from which the required executables can be built; on Linux, cmake will create a GNU Makefile that can be fed to the GNU
`make` program to compile and link.

It is advised that `cmake` be invoked with a `-B` directive on the command line to specify a subdirectory for writing project
build files to isolate different targets (i.e., Linux versus Windows) and without contaminating the source directory. For
example, on Windows, `-B msvc` may be used to write Visual Studio solution files under an _msvc_ directory, while on Linux
`-B gnu` may be used to write GNU Makefiles under a _gnu_ subdirectory.

Additionally, if boost is not installed such that the compiler can locate it in standard system paths, it is necessary to
inform CMake where boost headers and libraries can be found. In this case, using `-D` on the command line to define the
build variable **BOOST_ROOT** is an efficient solution.

### Examples

Assuming that the current directory is set to the correct location, and that boost is installed to D:\boost, the following
command on Windows will create a Visual Studio solutions file under the _msvc_ subdirectory and an executable under _Release_ within the _msvc_ subdirectory:

    cmake -B msvc -DBOOST_ROOT=D:\boost .
    cmake --build msvc --config Release
    
Assuming that the current directory is set to the correct location, and that boost is installed via package manager to
standard compiler include and library paths, the following command on Linux will create a makefile under the _gnu_ and then
compile the executable:

    cmake -B gnu .
    cd gnu
    make
    
## Running

By default, the resulting executable may be run from any subdirectory of the _src_ directory, as it will walk up the
directory tree in search of "../data" to locate data files for input.  This may be overridden by specifying an alternative
relative or absolute path via the `--directory` or `-d` command line switch; if the alternate is relative, it will be
applied at each level of the directory tree, walking upwards, until the work directory is resolved, whereas if it is an
absolute path, that location will determine the work directory unconditionally, regardless of where the program is run.

The resulting executable will run 4 trials by default.  This may be overridden by specifying a trial count parameter via
the `--trials` or `-t` command line switch.

As noted in the top-level README, zip files in the data directory source repository are expected to be extracted in-place
before code is run. (Note that for the similarity program, no deterministic input files are utilized by the C++
implementation.)

Program _standard output_ is used as the destination for a JSON-formatted structure representing timing metrics of the run,
while program _standard error_ is used for all other output, including progress messages and errors.
