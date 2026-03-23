# Advanced and Parallel Programming - Course Exercises

This repository contains the laboratory exercises for the "Advanced and Parallel Programming" (Programmazione Avanzata e Parallela) course. The projects are written in C and focus on memory management, algorithm optimization, CPU cache exploitation, and advanced build systems.

## 📂 Repository Structure

The repository is divided into different folders, each representing a specific laboratory session:

### [Lab 01: Project Structure, Makefiles, and Libraries](./lab01)
[cite_start]The goal of this exercise is to learn how to split a monolithic C project into multiple files with their respective headers and manage the build process[cite: 55]. 
* **Key Concepts:**
  * [cite_start]Splitting C source code into separate modules (e.g., `main.c`, `tree.c`, `node.c`)[cite: 57].
  * [cite_start]Writing an advanced `Makefile` to compile the project, set build options, and clean intermediate files[cite: 59, 60].
  * [cite_start]Creating a unified library (`libbst`) containing the implementation of a balanced binary search tree (BST)[cite: 61].
  * [cite_start]Using a `SHARED` variable in the Makefile to toggle between generating a **static** or **dynamic (shared)** library[cite: 62].
  * [cite_start]Implementing custom tree traversal for formatted printing[cite: 64, 66].

### [Lab 02: Branchless Programming and Mergesort](./lab02)
[cite_start]This exercise focuses on implementing the merge step of two sorted arrays of lengths $n_1$ and $n_2$ into a third array of length $n_1+n_2$[cite: 3].
* **Key Concepts:**
  * [cite_start]Writing two distinct implementations of the merge operation: one using standard `if/else` branches and one strictly **branchless**[cite: 7].
  * [cite_start]Understanding compiler optimizations by avoiding the generation of predicate instructions like `CMOV` (x86-64) or `CSEL` (ARM64) using GCC flags (`-fno-if-conversion`)[cite: 32].
  * [cite_start]Benchmarking execution times across 10 repetitions with random arrays of sizes ranging from 1,000 to 20,000 elements[cite: 22].
  * [cite_start]**Extra:** Expanding the code to implement a fully iterative (non-recursive) Mergesort algorithm[cite: 44, 49].

### [Lab 03: Cache Locality and Unrolled Linked Lists](./lab03)
This lab explores how hardware-level memory architecture (CPU Cache) impacts the performance of data structures that share the same $O(n)$ asymptotic complexity.
* **Key Concepts:**
  * Implementing a Standard Linked List and an **Unrolled Linked List** (grouping up to $k$ elements per node).
  * Dynamic memory allocation (`malloc`/`free`) for complex nested structures.
  * Measuring search performance to demonstrate the impact of **spatial locality** and cache hits/misses.

## 🛠️ Technologies & Tools
* **Language:** C (Standard C17)
* **Compiler:** GCC (with specific optimization flags like `-O3`, `-fno-if-conversion`)
* **Build System:** GNU Make

## 🚀 How to Build and Run
Each lab directory contains its own `Makefile`. To compile and run a specific exercise, navigate to its folder and use `make`:

```bash
cd lab01
make
./main
