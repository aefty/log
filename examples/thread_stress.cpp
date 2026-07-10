#include "logging.hpp"
#include <thread>
#include <vector>

// Spawns many threads hammering every log call concurrently and writes to a
// file. If output were not properly serialized, lines would interleave or
// get torn (e.g. a line missing its trailing '\n', or two messages fused on
// one line) and the line-count / prefix checks in verify.sh would fail.
int main() {
    logging::set_log_id("STRESS");
    logging::set_log_file("stress.log", /*append=*/false);
    logging::set_level(3);

    constexpr int kThreads = 16;
    constexpr int kIters = 500;

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([t] {
            for (int i = 0; i < kIters; ++i) {
                logging::info1("thread {} info1 iter {}", t, i);
                logging::info2("thread {} info2 iter {}", t, i);
                logging::info3("thread {} info3 iter {}", t, i);
                logging::warning("thread {} warning iter {}", t, i);
                try {
                    logging::error("thread {} error iter {}", t, i);
                } catch (const std::runtime_error &) {
                    // expected
                }
            }
        });
    }
    for (auto &th : threads)
        th.join();

    logging::close_log_file();
    return 0;
}
