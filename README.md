# log

A tiny, single-header C++17 logging library. No dependencies, no build step — just `#include "logging.hpp"`.

## Features

- Header-only: `include/logging.hpp`
- Five log calls: `info1`, `info2`, `info3`, `warning`, `error`
- Global verbosity level (-1/0/1/2/3) gates `info1`/`info2`/`info3`/`warning`; `-1` also silences `warning`; `error` always prints
- `error(...)` prints the message and then throws `std::runtime_error`
- Color-coded output: green (`info1`), teal (`info2`), violet (`info3`), yellow (`warning`), red (`error`)
- Line format: `[LOG_ID] [epoch_ms] [LEVEL] | message`
- Optional mirroring of all output to a log file (no ANSI color codes in the file)
- `{}`-style placeholder formatting, e.g. `logging::info1("x = {}, y = {}", x, y)`
  - `bool` prints as `True`/`False`
  - integers print as plain ints
  - floating-point values print in scientific notation (`1.123456e+02`); digit count is
    configurable with `logging::set_significant_digits(n)` (default 6)
- Thread-safe: safe to call from multiple threads concurrently, output is never interleaved or torn

## Usage

```cpp
#include "logging.hpp"

int main() {
    logging::set_log_id("MYAPP");        // optional, defaults to "LOG"
    logging::set_log_file("run.log");    // optional, mirrors output to a file
    logging::set_level(3);               // -1 = silent (errors only), 0 = no info output, 1 = info1, 2 = info1+info2, 3 = info1+info2+info3

    logging::info1("max_iter_mu = {}, max_iter_Theta = {}, max_iter_v = {}",
                    max_iter_mu, max_iter_Theta, max_iter_v);
    logging::info2("inner loop k = {}", k);

    logging::set_significant_digits(3);        // optional, defaults to 6
    logging::info3("gradient norm = {}, converged = {}", g, false);
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
[MYAPP] [1783700131163] [INFO1] | max_iter_mu = 10, max_iter_Theta = 20, max_iter_v = 30
[MYAPP] [1783700131163] [INFO2] | inner loop k = 5
[MYAPP] [1783700131164] [INFO3] | gradient norm = 3.100e-03, converged = False
[MYAPP] [1783700131164] [WARNG] | value 42 is out of expected range [0, 10]
[MYAPP] [1783700131164] [ERROR] | failed to converge after 100 iterations
```

- `INFO1` — green
- `INFO2` — teal
- `INFO3` — violet
- `WARNG` — yellow
- `ERROR` — red (and also throws `std::runtime_error` after printing)

## Argument formatting

Each `{}` placeholder is formatted according to the argument's type:

| Type | Output |
| --- | --- |
| `bool` | `True` / `False` |
| any integer type | plain integer, e.g. `42` |
| `float` / `double` | scientific notation, e.g. `3.100e-03` |
| everything else | streamed via `operator<<`, unchanged |

`logging::set_significant_digits(n)` controls how many digits follow the decimal point
in the mantissa of floating-point output (default `6`, i.e. `1.123456e+02`). It applies
to all subsequent log calls across all levels.

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
