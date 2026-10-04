#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace continuum_lob {

namespace {
std::string timestamp() {
  using clock = std::chrono::system_clock;
  auto now = clock::now();
  auto t = clock::to_time_t(now);
  std::tm tm{};
#if defined(_WIN32)
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}
}  // namespace

void log_info(const std::string& component, const std::string& msg) {
  std::cout << "[" << timestamp() << "] [INFO] [" << component << "] " << msg << "\n";
}

void log_warn(const std::string& component, const std::string& msg) {
  std::cerr << "[" << timestamp() << "] [WARN] [" << component << "] " << msg << "\n";
}

void log_error(const std::string& component, const std::string& msg) {
  std::cerr << "[" << timestamp() << "] [ERROR] [" << component << "] " << msg << "\n";
}

}  // namespace continuum_lob
