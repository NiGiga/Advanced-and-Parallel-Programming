# Advanced and Parallel Programming - Course Exercises

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)
![Bash](https://img.shields.io/badge/bash-%23121011.svg?style=for-the-badge&logo=gnu-bash&logoColor=white)
![Status](https://img.shields.io/badge/Status-In%20Progress-yellow)

**University:** Università degli Studi di Trieste (Units)  
**Course:** Advanced and Parallel Programming 
**Academic Year:** 2025-2026  

This repository contains the laboratory exercises for the "Advanced and Parallel Programming" (Programmazione Avanzata e Parallela) course. The projects are written in C and focus on memory management, algorithm optimization, CPU cache exploitation, and advanced build systems.

## 📂 Repository Structure

The repository is divided into different folders, each representing a specific laboratory session:

### [Lab 06: Project Structure, Makefiles, and Libraries](./Lecture 01)
[cite_start]The goal of this exercise is to learn how to split a monolithic C project into multiple files with their respective headers and manage the build process[cite: 55]. 
* **Key Concepts:**
  * [cite_start]Splitting C source code into separate modules (e.g., `main.c`, `tree.c`, `node.c`)[cite: 57].
  * [cite_start]Writing an advanced `Makefile` to compile the project, set build options, and clean intermediate files[cite: 59, 60].
  * [cite_start]Creating a unified library (`libbst`) containing the implementation of a balanced binary search tree (BST)[cite: 61].
  * [cite_start]Using a `SHARED` variable in the Makefile to toggle between generating a **static** or **dynamic (shared)** library[cite: 62].
  * [cite_start]Implementing custom tree traversal for formatted printing[cite: 64, 66].

### [Lab 08: Branchless Programming and Mergesort](./Lecture 02)
[cite_start]This exercise focuses on implementing the merge step of two sorted arrays of lengths $n_1$ and $n_2$ into a third array of length $n_1+n_2$[cite: 3].
* **Key Concepts:**
  * [cite_start]Writing two distinct implementations of the merge operation: one using standard `if/else` branches and one strictly **branchless**[cite: 7].
  * [cite_start]Understanding compiler optimizations by avoiding the generation of predicate instructions like `CMOV` (x86-64) or `CSEL` (ARM64) using GCC flags (`-fno-if-conversion`)[cite: 32].
  * [cite_start]Benchmarking execution times across 10 repetitions with random arrays of sizes ranging from 1,000 to 20,000 elements[cite: 22].
  * [cite_start]**Extra:** Expanding the code to implement a fully iterative (non-recursive) Mergesort algorithm[cite: 44, 49].

### [Lab 11: Cache Locality and Unrolled Linked Lists](./Lecture 03)
This lab explores how hardware-level memory architecture (CPU Cache) impacts the performance of data structures that share the same $O(n)$ asymptotic complexity.
* **Key Concepts:**
  * Implementing a Standard Linked List and an **Unrolled Linked List** (grouping up to $k$ elements per node).
  * Dynamic memory allocation (`malloc`/`free`) for complex nested structures.
  * Measuring search performance to demonstrate the impact of **spatial locality** and cache hits/misses.

### [Lab 14: N-ary Search Tree Implementation](./Lectur 04)
* **Key Concepts:**
  * Built an N-ary search tree: We implemented a custom search tree in C to function as a map, associating integer keys with float values.
  * Implemented insertion (cinsert): We wrote the logic to allocate new nodes, insert keys, and maintain them in ascending order within the node's array.
  * Implemented search (csearch): We created a function to navigate the tree's keys and children to find specific values.
  * Implemented visualization (print_ctree): We built a recursive function to print the tree's hierarchical structure using a specific bracketed format.
* **The core objectives:**
  * Analyze higher fanout: The main goal was to see the effect of using a search tree with a larger number of children per node.
  * Improve memory locality: This approach demonstrates how to build data structures that interact more efficiently with memory.
  * Understand advanced structures: This exercise serves as a foundation for understanding complex data structures like B-trees, which are essential for working with mass storage

### [Lab 18: Matrix Multiplication and Cache Locality](./Lecture 05)
This exercise focuses on implementing different variants of $n \times n$ floating-point matrix multiplication to explore the impact of memory access patterns and CPU cache optimization.
* **Key Concepts:**
  * Using a standard row-by-column naive multiplication to establish a performance baseline.
  * Developing a `transposed_multiply` approach where the second matrix is converted to a **column-major** layout, improving spatial locality during continuous memory reads by adjusting the index calculation (`j * n + k`).
  * Writing a robust computational `kernel` to multiply smaller sub-matrices, carefully managing logical boundary conditions (`i < x+dx && i < n`) to prevent out-of-bounds memory access.
  * Implementing a **blocked multiplication** (`blocked_multiply`) that processes matrices in fixed-size memory blocks (e.g., 16x16) to maximize cache hits and minimize RAM fetches.
  * Benchmarking and comparing execution times to verify the theoretical performance benefits of cache-aware algorithms (e.g., demonstrating a time reduction from ~639 ms to ~411 ms).

### [Lab 21: Image Manipulation, I/O in C](./Lecture 05)
The purpose of this exercise is to familiarize you with the various input/output methods
provided by the C language. Specifically, the goal is to create a system
for reading and modifying a very simple image format (NetPBM in
grayscale).

### [Lab 23: Image Manipulation and parallel programming](./Lecture 05)
The purpose of this exercise is to begin familiarizing yourself
with parallelizing C code using OpenMP. Specifically, by revisiting
the project for reading and writing images in NetPBM format covered
in the previous exercise, you are asked to generate transformations based
on a reference image. The idea is to treat each pixel as
a point in a matrix and apply a transformation to this point that


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
