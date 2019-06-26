# MastermindSolver

This project gathers a mastermind solver, and a small multithreaded benchmark, both written in C++.
The number of colors and pegs is fully customizable, as long as they are strictly positive integers.

The solver suggests a solution, for which you give back the number of right colors at the right place (red pegs), and the number of right colors at the wrong place (white pegs).
This goes over as long as the solver can't find the solution. You can try to give it wrong information, but it will detect it if this leads to no solution.

In benchmark mode, you can also change the number of threads, the batch size for each thread and the total number of games to play.
At the end, you will get the average number of trials to find the solution (4.66 for 4 pegs and 6 colors), and the time it took for all threads to finish.

# Requirements to compile

This project uses the [boost](https://www.boost.org/users/download/) library. You don't need to build boost before building this project.
However, you will need to change the following line `set includes= /I ../Includes/boost_1_70_0` in `build.bat` to something like `set includes= /I path/to/boost/library`.

# To do

I plan to optimize the benchmark with assembly code to make it faster.

I would also like to try compilation on the fly. This would allow to replace variables with constants and make some optimizations, especially by doing loop unrolling. For the moment, I am doing experiments with [TCC](https://bellard.org/tcc/).