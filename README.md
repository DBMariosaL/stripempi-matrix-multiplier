# StripeMPI Matrix Multiplier

Parallel matrix multiplication assignment implemented with MPI and stripe-based work distribution.

## Project overview

This project multiplies two square matrices in parallel across multiple MPI processes.
The coordinator process loads matrix data from binary files, broadcasts shared data,
scatters matrix stripes to workers, and gathers the final result.

Current code expects 8x8 matrix data files (`64` integers) and works best when the
number of MPI processes divides `8`.

## Clean structure

```text
.
|-- src/
|   `-- main.cpp
|-- tools/
|   `-- matrix_generator.cpp
|-- data/
|   |-- matA.dat
|   `-- matB.dat
|-- docs/
|   `-- assignment_notes.docx
|-- archive/
|   `-- legacy_test.cpp
`-- README.md
```

## Build

This project is **most straightforward on Linux** (and that is the usual target for MPI coursework/HPC clusters), but it is not Linux-only.

Platform notes:

- **Linux**: best-supported path (OpenMPI/MPICH via package manager).
- **macOS**: works with Homebrew OpenMPI.
- **Windows**: possible, but setup is usually harder; **WSL2 + Ubuntu + OpenMPI** is the easiest path.

### 1) Install MPI toolchain

Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y build-essential openmpi-bin libopenmpi-dev
```

Fedora:

```bash
sudo dnf install -y gcc-c++ openmpi openmpi-devel
```

Arch Linux:

```bash
sudo pacman -S --needed base-devel openmpi
```

macOS (Homebrew):

```bash
brew install open-mpi
```

### 2) Verify installation

```bash
mpic++ --version
mpirun --version
```

### 3) Compile

```bash
mkdir -p bin
mpic++ src/main.cpp -O2 -std=c++11 -o bin/stripe_mpi
```

## Run

```bash
mpirun -np 4 ./bin/stripe_mpi data/matA.dat data/matB.dat 8
```

Arguments:

- `argv[1]`: path to matrix A file
- `argv[2]`: path to matrix B file
- `argv[3]`: matrix dimension (use `8` with current data/generator)

## Generate sample matrix files

`tools/matrix_generator.cpp` can be used to generate matrix data files (currently writes
`matB.dat` by default).

## Notes

- Duplicate copies and compiled artifacts were removed to keep the repository clean.
- Legacy experimentation code is kept in `archive/legacy_test.cpp`.
