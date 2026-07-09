# log

A tiny, single-header C++17 logging library. No dependencies, no build step — just `#include "logging.hpp"`.

## Features

- Header-only: `include/logging.hpp`
- Four log calls: `info1`, `info2`, `warning`, `error`
- Global verbosity level (0/1/2) gates `info1`/`info2`; `warning` and `error` always print
- `error(...)` prints the message and then throws `std::runtime_error`
- Color-coded output: green (`info1`), teal (`info2`), yellow (`warning`), red (`error`)
- Line format: `[LOG_ID] [time,date,millisecond] [LEVEL] | message`
- Optional mirroring of all output to a log file (no ANSI color codes in the file)
- `{}`-style placeholder formatting, e.g. `logging::info1("x = {}, y = {}", x, y)`
- Thread-safe: safe to call from multiple threads concurrently, output is never interleaved or torn

## Usage

```cpp
#include "logging.hpp"

int main() {
    logging::set_log_id("MYAPP");        // optional, defaults to "LOG"
    logging::set_log_file("run.log");    // optional, mirrors output to a file
    logging::set_level(2);               // 0 = no info output, 1 = info1 only, 2 = info1 + info2

    logging::info1("max_iter_mu = {}, max_iter_Theta = {}, max_iter_v = {}",
                    max_iter_mu, max_iter_Theta, max_iter_v);
    logging::info2("inner loop k = {}", k);
    logging::warning("value {} is out of range", v);

    logging::error("failed to converge after {} iterations", n); // prints, then throws std::runtime_error
}
```

Build the demo:

```sh
g++ -std=c++17 -Iinclude examples/demo.cpp -o demo && ./demo
```

## Output

Console output is color-coded (green/teal/yellow/red); the same lines are written to the
log file, if configured, without the color codes:

```
[MYAPP] [21:29:37,2026-07-09,315] [INFO1] | max_iter_mu = 10, max_iter_Theta = 20, max_iter_v = 30
[MYAPP] [21:29:37,2026-07-09,315] [INFO2] | inner loop k = 5
[MYAPP] [21:29:37,2026-07-09,315] [WARNING] | value 42 is out of expected range [0, 10]
[MYAPP] [21:29:37,2026-07-09,315] [ERROR] | failed to converge after 100 iterations
```

- `INFO1` — green
- `INFO2` — teal
- `WARNING` — yellow
- `ERROR` — red (and also throws `std::runtime_error` after printing)

## Thread safety

All state (verbosity level, log id, log file) can be read and written concurrently from
multiple threads. The level is a `std::atomic<int>` so `info1()`/`info2()` can check it
without locking; writing to stderr/the log file is serialized through a single mutex, so
lines from concurrent log calls are always complete and never interleaved.

`examples/thread_stress.cpp` exercises this with 16 threads x 500 iterations x 4 log
calls each and verifies every line in the output is well-formed:

```sh
g++ -std=c++17 -pthread -Iinclude examples/thread_stress.cpp -o thread_stress && ./thread_stress
```
