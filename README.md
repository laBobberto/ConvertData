This project is dedicated to the study and optimization of the algorithm for converting a Gregorian date to an ISO 8601 week number. The primary focus is on performance across legacy architectures and the impact of various compiler optimization levels.

## Study in `new_test_cases`

This subdirectory contains a detailed comparison of four algorithm implementations. All tests were conducted using the `i486-linux-musl-g++` cross-compiler (GCC 11.2) for the **i586** architecture.

### Tested Functions:

1.  **Original** (`version_original.cpp`): 
    *   A classic implementation using standard `if-else` blocks and logical checks.
    *   Contains multiple branches, which can lead to pipeline stalls on older processors.

2.  **V1_EarlyReturn** (`version_v1.cpp`):
    *   Optimized using "early returns" to exit the function as soon as the result is known.
    *   Reduces the amount of code executed in simple cases but retains logical transitions.

3.  **V2_BitOps** (`version_v2.cpp`):
    *   Optimized through bitwise operations (shifts, masks).
    *   Minimizes branching by replacing logic with arithmetic and bit manipulation.

4.  **V4_MathMask** (`version_v4.cpp`):
    *   Extreme optimization completely avoiding branches (`branchless`).
    *   Uses mathematical masks (the `0` or `1` result of logical comparisons) and multiplications to select the final value without `if` statements.
    *   Demonstrated the best performance results, especially at lower optimization levels.

### Testing Parameters:
*   **Standard**: C++20
*   **Architecture**: `-march=i586 -mtune=i686`
*   **Flags**: `-O1`, `-O2`, `-O3`, `-fno-omit-frame-pointer`, `-static`
*   **Data**: 1,000,000 iterations on random dates within the range 1800-3000 AD.

## Results
All reports and visualizations are located in the `new_test_cases/` folder:
*   `results.csv`: Summary table (Average, Median, Min, P95, P99).
*   `i586_benchmark_charts_combined.png`: Comparative charts for execution time and speedup.

---
*This project was prepared as part of a study on C++ micro-optimizations.*
