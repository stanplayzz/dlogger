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
#include <mutex>
#include <source_location>
#include <string>

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
} // namespace dlog
