/*
MIT License

Copyright (c) 2026 StanPlayzz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace dlog {
namespace fs = std::filesystem;

enum class Level : std::uint8_t {
	INFO,
	DEBUG,
	WARNING,
	ERROR,
};

class Logger {
  public:
	// Console Only Logger
	explicit Logger(Level min_level = Level::INFO) : m_min_level(min_level) {}

	// Console + Save to File Logger
	Logger(std::string const& log_dir, Level min_level = Level::INFO) : m_min_level(min_level), m_to_file(true) {
		fs::create_directories(log_dir);

		auto ts = timestamp();
		m_path = fs::path(log_dir) / (ts + ".log");
		auto symlink = fs::path(log_dir) / "latest.log";

		if (fs::is_symlink(symlink)) { fs::remove(symlink); }

		m_file.open(m_path, std::ios::app);
		if (!m_file.is_open()) {
			std::cerr << "[LOGGER] Could not open log file: " << m_path << "\n";
			m_to_file = false;
			return;
		}

#ifndef _WIN32
		fs::create_symlink(m_path.filename(), symlink);
#endif
	}

	~Logger() {
		if (m_file.is_open()) { m_file.close(); }
	}

	Logger(Logger const&) = delete;
	Logger& operator=(Logger const&) = delete;

	void log(std::string const& message, std::source_location loc = std::source_location::current()) {
		write(Level::INFO, message, loc.file_name(), loc.line());
	}

	void debug(std::string const& message, std::source_location loc = std::source_location::current()) {
		write(Level::DEBUG, message, loc.file_name(), loc.line());
	}

	void warn(std::string const& message, std::source_location loc = std::source_location::current()) {
		write(Level::WARNING, message, loc.file_name(), loc.line());
	}

	void error(std::string const& message, std::source_location loc = std::source_location::current()) {
		write(Level::ERROR, message, loc.file_name(), loc.line());
	}

  private:
	Level m_min_level{};
	bool m_to_file{};
	std::ofstream m_file{};
	fs::path m_path{};
	std::mutex m_mutex{};

	static std::string timestamp() {
		auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
		return std::format("{:%Y-%m-%d_%H:%M:%S}", now);
	}

	void write(Level level, std::string const& message, char const* file, int line) {
		if (level < m_min_level) { return; }

		auto ts = timestamp();

		std::string tag{};
		std::string color{};
		switch (level) {
		case Level::INFO:
			tag = "INFO";
			color = "\033[37m";
			break;
		case Level::DEBUG:
			tag = "DEBUG";
			color = "\033[36m";
			break;
		case Level::WARNING:
			tag = "WARNING";
			color = "\033[33m";
			break;
		case Level::ERROR:
			tag = "ERROR";
			color = "\033[31m";
			break;
		}

		auto location = (line >= 0) ? std::format("  ({}:{})", file, line) : "";
		auto plain = std::format("[{}] [{}] {}{}\n", ts, tag, message, location);
		std::lock_guard<std::mutex> lock{m_mutex};

		auto& out = (level == Level::ERROR) ? std::cerr : std::cout;
		out << std::format("{}{}\033[0m", color, plain);

		if (m_to_file && m_file.is_open()) { m_file << plain; }
	}
};

namespace detail {
inline std::unique_ptr<Logger>& default_logger_storage() {
	static std::unique_ptr<Logger> logger = std::make_unique<Logger>();
	return logger;
}
inline std::mutex& default_logger_mutex() {
	static std::mutex mutex;
	return mutex;
}
} // namespace detail

inline Logger& default_logger() { return *detail::default_logger_storage(); }

inline void set_default_log_directory(std::string const& log_dir, Level min_level = Level::INFO) {
	auto logger = std::make_unique<Logger>(log_dir, min_level);
	std::lock_guard<std::mutex> lock{detail::default_logger_mutex()};
	detail::default_logger_storage() = std::move(logger);
}

namespace detail {
template <typename T>
consteval std::string_view type_name() {
#if defined(__clang__)
	std::string_view name = __PRETTY_FUNCTION__;
	auto const start = name.find("T = ") + 4;
	return name.substr(start, name.find(']', start) - start);
#elif defined(__GNUC__)
	std::string_view name = __PRETTY_FUNCTION__;
	auto const start = name.find("with T = ") + 9;
	return name.substr(start, name.find(';', start) - start);
#elif defined(_MSC_VER)
	std::string_view name = __FUNCSIG__;
	auto const start = name.find("type_name<") + 10;
	return name.substr(start, name.find(">(void)", start) - start);
#else
	return "unknown";
#endif
}
} // namespace detail

template <typename Type>
class TypedLogger {
  public:
	void log(std::string const& message, std::source_location loc = std::source_location::current()) const {
		default_logger().log(tagged(message), loc);
	}
	void debug(std::string const& message, std::source_location loc = std::source_location::current()) const {
		default_logger().debug(tagged(message), loc);
	}
	void warn(std::string const& message, std::source_location loc = std::source_location::current()) const {
		default_logger().warn(tagged(message), loc);
	}
	void error(std::string const& message, std::source_location loc = std::source_location::current()) const {
		default_logger().error(tagged(message), loc);
	}

  private:
	static std::string tagged(std::string const& message) {
		return std::format("[{}] {}", detail::type_name<Type>(), message);
	}
};
} // namespace dlog
