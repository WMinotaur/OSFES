Prime Numbers - Windows API version
=====================================
Tasks 200-207, Part 2 of "Operating systems for embedded systems" (course 1060).

This version uses the Windows API directly:
  CreateThread / WaitForSingleObject / WaitForMultipleObjects / GetExitCodeThread
  CreateMutex / ReleaseMutex / CloseHandle
  GetSystemInfo

== Building on Windows ==

Option A: MinGW-w64 (recommended for the lab devcontainer with MinGW)
----------------------------------------------------------------------
  cmake --preset mingw
  cmake --build build/mingw --config Release

  Or directly without cmake:
  x86_64-w64-mingw32-g++ -std=c++20 -O2 -Wall -o prime_numbers.exe src/main.cpp

Option B: Visual Studio / MSVC
----------------------------------------------------------------------
  cmake --preset msvc
  cmake --build build/msvc --config Release

  The resulting executable is in build/msvc/Release/prime_numbers.exe

Option C: MSYS2 / MinGW shell on Windows
----------------------------------------------------------------------
  g++ -std=c++20 -O2 -Wall -o prime_numbers.exe src/main.cpp

== Running ==

  prime_numbers.exe

== Expected output ==

  === Prime Numbers - Operating Systems for Embedded Systems ===
  Logical processors: <N>

  --- Task 201: isPrime self-test ---
  All self-tests passed.
  Done.

  --- Task 202: Single worker thread ---
  Found 78498 prime numbers.
  Duration: ~XXX ms.

  --- Task 203: Performance vs. thread count ---
  Threads: 1  Duration: XXX ms
  Threads: 2  Duration: XXX ms
  ...

  --- Tasks 204-206: Dynamic workload assignment (n=10000000) ---
  Threads: 1
    Found 664579 primes.
    Duration: XXXX ms
  ...

  === Done ===
