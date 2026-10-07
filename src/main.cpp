/**
 * Prime Numbers - multithreaded primality test
 *
 * Tasks 200-207 from "Operating systems for embedded systems" course (Part 2).
 * Windows API version: uses CreateThread, WaitForMultipleObjects, CreateMutex
 * as described in the course materials.
 */

#include <iostream>
#include <cstdint>
#include <cmath>
#include <chrono>
#include <vector>
#include <windows.h>

using namespace std::chrono_literals;

// ============================================================
// Task 201: Number primality test
// ============================================================

/**
 * Checks if the given number n is prime (naive trial-division test).
 * @param[in] n number to check
 * @retval true  if the number is prime
 * @retval false if the number is NOT prime
 */
static bool isPrime(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;

    for (uint64_t div = 3; div <= static_cast<uint64_t>(std::sqrt(static_cast<double>(n))); div += 2) {
        if (n % div == 0) {
            return false;
        }
    }
    return true;
}

/**
 * Runs basic self-tests of isPrime() as required by Task 201.
 */
static void runTask201SelfTest() {
    std::cout << "\n--- Task 201: isPrime self-test ---" << std::endl;
    bool ok = true;
    if (true  == isPrime(1))  { std::cout << "1 IS NOT a prime number. Check your implementation" << std::endl; ok = false; }
    if (false == isPrime(2))  { std::cout << "2 IS a prime number. Check your implementation"     << std::endl; ok = false; }
    if (false == isPrime(3))  { std::cout << "3 IS a prime number. Check your implementation"     << std::endl; ok = false; }
    if (true  == isPrime(4))  { std::cout << "4 IS NOT a prime number. Check your implementation" << std::endl; ok = false; }
    if (false == isPrime(5))  { std::cout << "5 IS a prime number. Check your implementation"     << std::endl; ok = false; }
    if (true  == isPrime(6))  { std::cout << "6 IS NOT a prime number. Check your implementation" << std::endl; ok = false; }
    if (false == isPrime(97)) { std::cout << "97 IS a prime number. Check your implementation"    << std::endl; ok = false; }
    if (ok) {
        std::cout << "All self-tests passed." << std::endl;
    }
    std::cout << "Done." << std::endl;
}

// ============================================================
// Task 202: Worker thread that runs primality tests
// ============================================================

/**
 * Defines a range of numbers for a single worker thread to test.
 * Thread t (1-based) tests numbers: lower, lower+increment, lower+2*increment, ...
 */
struct Range {
    uint64_t lower;     ///< lower number range boundary
    uint64_t upper;     ///< upper number range boundary
    uint64_t increment; ///< how many numbers to skip between checks
    uint64_t result;    ///< output: number of primes found (written by the thread)
};

/**
 * Windows thread function for Task 202 / Task 203.
 * Tests primality for numbers defined by the Range structure.
 * Returns the count of primes found as the thread exit code.
 * Note: DWORD is 32-bit so we cap at MAXDWORD; for large ranges use result field.
 */
DWORD WINAPI primeTestThread(void* args) {
    Range* range = static_cast<Range*>(args);
    uint64_t count = 0;

    for (uint64_t n = range->lower; n <= range->upper; n += range->increment) {
        if (isPrime(n)) {
            ++count;
        }
    }

    range->result = count;
    // Return lower 32 bits via exit code (as shown in the course)
    return static_cast<DWORD>(count);
}

/**
 * Runs Task 202: single worker thread over [2, 1000000], measures time.
 */
static void runTask202() {
    std::cout << "\n--- Task 202: Single worker thread ---" << std::endl;

    Range range{2, 1000000, 1, 0};

    auto startTime = std::chrono::system_clock::now();

    // Create and run the thread
    HANDLE handle = CreateThread(nullptr, 0, primeTestThread, &range, 0, nullptr);

    // Wait for the thread to finish
    WaitForSingleObject(handle, INFINITE);

    auto endTime = std::chrono::system_clock::now();

    // Get thread result
    DWORD result;
    GetExitCodeThread(handle, &result);
    CloseHandle(handle);

    std::cout << "Found " << result << " prime numbers." << std::endl;
    auto duration = endTime - startTime;
    std::cout << "Duration: "
              << std::chrono::duration<double, std::milli>(duration).count()
              << " ms." << std::endl;
}

// ============================================================
// Task 203: Checking performance of a multithreaded application
// ============================================================

/**
 * Runs a multithreaded prime number test up to provided number n,
 * using the provided number of threads (interleaved workload split).
 * Each thread t (1-based) tests: t, t+threads, t+2*threads, ...
 * @param[in] n       maximum number to check
 * @param[in] threads number of threads to use
 * @return duration in milliseconds
 */
static double runPrimeTest(uint64_t n, unsigned int threads) {
    std::vector<Range>  ranges(threads);
    std::vector<HANDLE> handles(threads);

    // For thread t (1-based): lower=t, upper=n, increment=threads
    for (unsigned int t = 1; t <= threads; ++t) {
        ranges[t - 1] = Range{t, n, threads, 0};
    }

    auto startTime = std::chrono::system_clock::now();

    for (unsigned int t = 0; t < threads; ++t) {
        handles[t] = CreateThread(nullptr, 0, primeTestThread, &ranges[t], 0, nullptr);
    }

    // Wait for all threads to finish
    WaitForMultipleObjects(static_cast<DWORD>(threads), handles.data(), TRUE, INFINITE);

    auto endTime = std::chrono::system_clock::now();

    // Collect results and close handles
    uint64_t total = 0;
    for (unsigned int t = 0; t < threads; ++t) {
        total += ranges[t].result;
        CloseHandle(handles[t]);
    }

    std::cout << "  Primes found: " << total << std::endl;
    return std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

/**
 * Runs Task 203: benchmark with 1..maxThreads threads.
 */
static void runTask203() {
    std::cout << "\n--- Task 203: Performance vs. thread count ---" << std::endl;
    const uint64_t N = 1000000;
    // Use number of logical processors as upper bound.
    // Cap at MAXIMUM_WAIT_OBJECTS (64) - WaitForMultipleObjects limit.
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    unsigned int maxThreads = min(sysInfo.dwNumberOfProcessors * 2, (DWORD)MAXIMUM_WAIT_OBJECTS);

    for (unsigned int t = 1; t <= maxThreads; ++t) {
        double ms = runPrimeTest(N, t);
        std::cout << "Threads: " << t << "  Duration: " << ms << " ms" << std::endl;
    }
}

// ============================================================
// Tasks 204-206: Dynamic workload assignment with mutex
// ============================================================

/// Upper limit for the improved workload assignment solution
static const uint64_t NUMBER_LIMIT = 10ULL * 1000 * 1000;

/**
 * Shared state for the dynamic workload dispatcher.
 * Protected by a Windows HANDLE mutex.
 */
struct Dispatcher {
    uint64_t next;        ///< next number to be assigned
    uint64_t upper;       ///< upper limit (inclusive)
    uint64_t chunkSize;   ///< current chunk size (adapted dynamically)
    uint64_t totalPrimes; ///< accumulated prime count from all threads
    HANDLE   mutex;       ///< Windows mutex protecting this struct
};

/**
 * Acquires the next chunk of work from the dispatcher.
 * @param[in]  d     pointer to the shared Dispatcher
 * @param[out] lower first number of the chunk
 * @param[out] upper last number of the chunk (inclusive)
 * @return true if a chunk was assigned, false if all work is done
 */
static bool getNextChunk(Dispatcher* d, uint64_t& lower, uint64_t& upper) {
    WaitForSingleObject(d->mutex, INFINITE);

    bool hasWork = false;
    if (d->next <= d->upper) {
        lower    = d->next;
        upper    = min(d->next + d->chunkSize - 1, d->upper);
        d->next  = upper + 1;
        hasWork  = true;
    }

    ReleaseMutex(d->mutex);
    return hasWork;
}

/**
 * Reports chunk processing time to the dispatcher for adaptive sizing.
 * Scales chunk size up/down around a ~50 ms target.
 */
static void reportChunkDuration(Dispatcher* d, double ms) {
    const double targetMs = 50.0;

    WaitForSingleObject(d->mutex, INFINITE);

    if (ms > targetMs * 2.0 && d->chunkSize > 1) {
        d->chunkSize = max(UINT64_C(1), d->chunkSize / 2);
    } else if (ms < targetMs / 2.0) {
        d->chunkSize = d->chunkSize * 2;
    }

    ReleaseMutex(d->mutex);
}

/**
 * Windows thread function for Tasks 204-206.
 * Continuously acquires work chunks, processes them and reports back timing.
 */
DWORD WINAPI dynamicWorkerThread(void* args) {
    Dispatcher* d = static_cast<Dispatcher*>(args);
    uint64_t lower = 0;
    uint64_t upper = 0;
    uint64_t localCount = 0;

    while (getNextChunk(d, lower, upper)) {
        auto chunkStart = std::chrono::steady_clock::now();

        for (uint64_t n = lower; n <= upper; ++n) {
            if (isPrime(n)) {
                ++localCount;
            }
        }

        auto chunkEnd = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(chunkEnd - chunkStart).count();
        reportChunkDuration(d, ms);
    }

    // Add local count to total under mutex protection
    WaitForSingleObject(d->mutex, INFINITE);
    d->totalPrimes += localCount;
    ReleaseMutex(d->mutex);

    return 0;
}

/**
 * Runs Tasks 204-206: dynamic workload assignment for a given number of threads.
 * @param[in] threads number of worker threads
 * @return duration in milliseconds
 */
static double runDynamicPrimeTest(unsigned int threads) {
    Dispatcher d;
    d.next        = 2;
    d.upper       = NUMBER_LIMIT;
    d.chunkSize   = 500;
    d.totalPrimes = 0;
    d.mutex       = CreateMutex(nullptr, FALSE, nullptr);

    std::vector<HANDLE> handles(threads);

    auto startTime = std::chrono::system_clock::now();

    for (unsigned int t = 0; t < threads; ++t) {
        handles[t] = CreateThread(nullptr, 0, dynamicWorkerThread, &d, 0, nullptr);
    }

    WaitForMultipleObjects(static_cast<DWORD>(threads), handles.data(), TRUE, INFINITE);

    auto endTime = std::chrono::system_clock::now();

    for (unsigned int t = 0; t < threads; ++t) {
        CloseHandle(handles[t]);
    }
    CloseHandle(d.mutex);

    std::cout << "  Found " << d.totalPrimes << " primes." << std::endl;
    return std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

/**
 * Runs Tasks 204-206: benchmark dynamic workload assignment with 1..maxThreads threads.
 */
static void runTask204to206() {
    std::cout << "\n--- Tasks 204-206: Dynamic workload assignment (n=" << NUMBER_LIMIT << ") ---" << std::endl;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    // Cap at MAXIMUM_WAIT_OBJECTS (64) - WaitForMultipleObjects limit.
    unsigned int maxThreads = min(sysInfo.dwNumberOfProcessors * 2, (DWORD)MAXIMUM_WAIT_OBJECTS);

    for (unsigned int t = 1; t <= maxThreads; ++t) {
        std::cout << "Threads: " << t << std::endl;
        double ms = runDynamicPrimeTest(t);
        std::cout << "  Duration: " << ms << " ms" << std::endl;
    }
}

// ============================================================
// main
// ============================================================

int main() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::cout << "=== Prime Numbers - Operating Systems for Embedded Systems ===" << std::endl;
    std::cout << "Logical processors: " << sysInfo.dwNumberOfProcessors << std::endl;

    // Task 201: isPrime self-test
    runTask201SelfTest();

    // Task 202: single thread, measure time
    runTask202();

    // Task 203: interleaved split, multiple threads
    runTask203();

    // Tasks 204-206: dynamic workload assignment
    runTask204to206();

    std::cout << "\n=== Done ===" << std::endl;
    return 0;
}
