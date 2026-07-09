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
