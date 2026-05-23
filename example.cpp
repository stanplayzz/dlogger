#include "logger.hpp"

int main() {
	dlog::Logger logger{std::filesystem::current_path()};

	logger.log("Log message");
	logger.debug("Debug message");
	logger.warn("Warning message");
	logger.error("FIRE AAAAH");
}