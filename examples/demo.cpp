#include "logging.hpp"

int main() {
    logging::set_log_id("MYAPP");
    logging::set_log_file("run.log");
    logging::set_level(3);

    int max_iter_mu = 10, max_iter_Theta = 20, max_iter_v = 30;
    logging::info1("max_iter_mu = {}, max_iter_Theta = {}, max_iter_v = {}", max_iter_mu, max_iter_Theta, max_iter_v);
    logging::info2("inner loop k = {}", 5);
    logging::info3("gradient norm = {}", 0.0031);
    logging::warning("value {} is out of expected range [{}, {}]", 42, 0, 10);

    logging::set_level(0); // silence info, warning/error still print
    logging::info1("this will not print");

    try {
        logging::error("failed to converge after {} iterations", 100);
    } catch (const std::runtime_error &e) {
        logging::warning("caught expected exception: {}", e.what());
    }

    return 0;
}
