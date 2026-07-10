#pragma once

// Single-header C++ logging library.
//
// Usage:
//   logging::set_level(3);                 // 0 = no info output, 1 = info1, 2 = info1+info2, 3 = info1+info2+info3
//   logging::set_log_id("MYAPP");          // optional, defaults to "LOG"
//   logging::set_log_file("run.log");      // optional, also mirrors output to a file
//
//   logging::info1("max_iter_mu = {}, max_iter_Theta = {}", max_iter_mu, max_iter_Theta);
//   logging::info2("inner loop k = {}", k);
//   logging::info3("gradient norm = {}", g);
//   logging::warning("value {} out of range", v);
//   logging::error("failed to converge after {} iterations", n);  // prints then throws std::runtime_error
//
// warning() and error() are always printed regardless of the level set with set_level().
// error() throws std::runtime_error(message) after printing.

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>

// Thread safety: all state (level, log id, log file) is safe to read and
// write concurrently from multiple threads. The level is a std::atomic so
// info1()/info2() can check it without locking; the log id, log file and
// the actual write to stderr/file are serialized through a single mutex so
// lines from concurrent log calls are never interleaved or torn.

namespace logging {

namespace detail {

inline std::atomic<int> &log_level() {
    static std::atomic<int> level{2};
    return level;
}

inline std::string &log_id() {
    static std::string id = "LOG";
    return id;
}

inline std::ofstream &log_file() {
    static std::ofstream file;
    return file;
}

inline std::mutex &log_mutex() {
    static std::mutex m;
    return m;
}

inline std::string timestamp() {
    using namespace std::chrono;
    const auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return std::to_string(ms.count());
}

inline void format_impl(std::ostringstream &oss, const std::string &fmt, size_t pos) {
    oss << fmt.substr(pos);
}

template <typename T, typename... Rest>
inline void format_impl(std::ostringstream &oss, const std::string &fmt, size_t pos, T &&value, Rest &&...rest) {
    const size_t open = fmt.find("{}", pos);
    if (open == std::string::npos) {
        oss << fmt.substr(pos);
        return;
    }
    oss << fmt.substr(pos, open - pos) << value;
    format_impl(oss, fmt, open + 2, std::forward<Rest>(rest)...);
}

template <typename... Args>
inline std::string format(const std::string &fmt, Args &&...args) {
    std::ostringstream oss;
    format_impl(oss, fmt, 0, std::forward<Args>(args)...);
    return oss.str();
}

constexpr const char *COLOR_RESET = "\033[0m";
constexpr const char *COLOR_GREEN = "\033[32m";   // info1
constexpr const char *COLOR_TEAL = "\033[36m";    // info2
constexpr const char *COLOR_VIOLET = "\033[35m";  // info3
constexpr const char *COLOR_YELLOW = "\033[33m";  // warning
constexpr const char *COLOR_RED = "\033[31m";     // error

inline void emit(const char *level_str, const char *color, const std::string &msg) {
    std::lock_guard<std::mutex> lock(log_mutex());
    std::ostringstream line;
    line << "[" << log_id() << "] [" << timestamp() << "] [" << level_str << "] | " << msg;

    std::ostream &out = std::cerr;
    out << color << line.str() << COLOR_RESET << '\n';

    if (log_file().is_open()) {
        log_file() << line.str() << '\n';
        log_file().flush();
    }
}

} // namespace detail

// Sets the info verbosity: 0 = no info output, 1 = info1, 2 = info1 + info2, 3 = info1 + info2 + info3.
// Does not affect warning() or error(), which always print.
inline void set_level(int level) { detail::log_level().store(level); }

inline int get_level() { return detail::log_level().load(); }

// Sets the [LOG_ID] tag shown in every line. Defaults to "LOG".
inline void set_log_id(const std::string &id) {
    std::lock_guard<std::mutex> lock(detail::log_mutex());
    detail::log_id() = id;
}

// Mirrors all subsequent output to the given file, in addition to stderr.
inline void set_log_file(const std::string &path, bool append = true) {
    std::lock_guard<std::mutex> lock(detail::log_mutex());
    detail::log_file().open(path, append ? (std::ios::out | std::ios::app) : (std::ios::out | std::ios::trunc));
}

inline void close_log_file() {
    std::lock_guard<std::mutex> lock(detail::log_mutex());
    if (detail::log_file().is_open())
        detail::log_file().close();
}

template <typename... Args>
inline void info1(const std::string &fmt, Args &&...args) {
    if (detail::log_level().load() >= 1)
        detail::emit("INFO1", detail::COLOR_GREEN, detail::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
inline void info2(const std::string &fmt, Args &&...args) {
    if (detail::log_level().load() >= 2)
        detail::emit("INFO2", detail::COLOR_TEAL, detail::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
inline void info3(const std::string &fmt, Args &&...args) {
    if (detail::log_level().load() >= 3)
        detail::emit("INFO3", detail::COLOR_VIOLET, detail::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
inline void warning(const std::string &fmt, Args &&...args) {
    detail::emit("WARNG", detail::COLOR_YELLOW, detail::format(fmt, std::forward<Args>(args)...));
}

// Prints the message and then throws std::runtime_error(message).
template <typename... Args>
[[noreturn]] inline void error(const std::string &fmt, Args &&...args) {
    std::string msg = detail::format(fmt, std::forward<Args>(args)...);
    detail::emit("ERROR", detail::COLOR_RED, msg);
    throw std::runtime_error(msg);
}

} // namespace logging
