# s3030

Brute-forces all valid expressions to reach a target value  
Tries **permutations**, **operator arrangements**, and **expression shapes** unless specified otherwise

## Features

- Operator banning (exclude specific operators from the search)
- Optional number freezing (no permutations)

## Compilation

This program is written in standard C. No external dependencies are required

To compile:

```bash  
gcc -O2 -o s3030 s3030.c -Iinclude src/perms.c src/catalan.c src/pparser.c
```

## Usage Examples

```bash  
./s3030 1 2 3 4 -fan
```  
- -f: Freezes number order (no permutations)
- -a: Shows all matching expressions
- -n: Numbers each solution in the output

```bash
./s3030 1 2 3 4 5 6
```
- No flags: Permutes numbers, tries all operator combinations  
Stops at the first match and prints it

```bash
./s3030 4 4 4 4 + -a
```
- +: Bans addition (any operator given as a bare symbol is excluded)
- -a: Shows all matching expressions. Not optimistic

```bash  
./s3030 2 5 2 -f -t=9
```  
- -f: Freezes number order  
- -t=9: Sets the target value to 9  

> s3030 outputs expressions using postfix notation
>
> You can write flags as -f -a -n ... or merge any combination freely. Flag order does not matter
>
> See ./s3030 --help for more flags and information

## License

Copyright © 2025 Lucas Martín  
Distributed under the MIT License. See the [LICENSE](./LICENSE) file for full details

## Limitations

### Float comparison is strict  

If a result does not exactly match the target (bit-for-bit), it is discarded. Some tolerance can be applied using the `-e` flag; however, this only affects solution validation, not the generation of permutations

### Parallelism potential

The problem is highly parallelizable by nature. Adding it might be an option I explore at some point, and anyone is welcome to try it themselves

### Postfix representation defines identity  
The program builds and evaluates expressions in postfix form. Because of that, the grouping of operations determines the structure  
For example, expressions like `(1 + 2) + 3 + 4` and `1 + 2 + 3 + 4` share the same postfix (`1 2 + 3 + 4 +`) and are treated as equivalent  
However, `1 + (2 + 3) + 4` results in a different postfix (`1 2 3 + + 4 +`) and is therefore considered a different expression

### 64-bit limits and practical RAM boundaries  
All internal counting uses 64-bit unsigned integers. This allows safe computation up to `factorial(20)` and `C(33)` is the practical upper bound for Catalan paths: although `uint64_t` can hold `C(36)`, once multiplied by `(2n - 2)` the result overflows beyond `n = 33`. If you use `-f` (no permutation), you can input up to 34 numbers, resulting in 33 binary operations  
In practice, you’ll run out of memory long before hitting any of these theoretical limits. The combinatorial space grows quickly, and your RAM will wave the white flag long before the code does

For quick reference:  
| Numbers provided | Number of paths ($C_n$) | Bit storage required | RAM usage (GiB) |
|----|--------------:|---------------:|--------:|
| 17 |      35357670 |     1060730100 |    0.12 |
| 18 |     129644790 |     4148633280 |    0.48 |
| 19 |     477638700 |    16239715800 |    1.89 |
| 20 |    1767263190 |    63621474840 |    7.41 |
| 21 |    6564120420 |   249436575960 |   29.04 |
| 22 |   24466267020 |   978650680800 |  113.93 |
| 23 |   91482563640 |  3842267672880 |  447.30 |
| 24 |  343059613650 | 15094623000600 | 1757.25 |
| 25 | 1289904147324 | 59335590776904 | 6907.57 |

As a result, I didn’t find it worth the time integrating something like a BigInt library

### No detection of NaN or infinity  
If an expression results in NaN or $\pm \infty$ due to overflow, the program does not detect or filter it

### No progress feedback  
There is no bar, no counter, no animation. The program runs silently until it finds a result or exhausts the search

## Catalan Numbers and Postfix Notation

This program represents expressions in **postfix notation** (also known as Reverse Polish), where operators follow their operands and expression structure is fully determined by token order

To ensure only syntactically valid postfix expressions are generated, the program uses **Catalan numbers** as a guiding principle. For `n` binary operations, there are exactly `C(n)` valid expression shapes. These are encoded as **Catalan paths**, bit sequences that direct the placement of numbers and operators while maintaining structural balance at every step

Each Catalan path describes how to interleave the numbers and operations without ever producing an invalid intermediate expression

For more background on these beautiful numbers and sequences, and why they appear in so many unexpected places, please see:

- [OEIS A000108 – Catalan numbers](https://oeis.org/A000108)
- [Wikipedia – Catalan number](https://en.wikipedia.org/wiki/Catalan_number)
- [YouTube – *My favorite Sequence of Numbers* by Finnley](https://www.youtube.com/watch?v=X6NQMB6JaF0)

They show up in valid parentheses, binary trees, stack-sortable permutations, and now, in your search for 10

---

Regarding the name, **3030** has no functional meaning, but:

**3030** in base

$$\mathbb{B} = \sqrt[3]{\frac{15 + 2\sqrt{57}}{9}} - \frac{1}{\sqrt[3]{3(15 + 2\sqrt{57})}}$$  

is **exactly 10**

You probably didn’t know that. You do now

Special thanks to ChatGPT for contributing heavily to the writing of this README

Lucas M.
