# Dumb Logger
Header only library to output logs to console and file.

## Usage
```cpp
// Console only
dlog::Logger logger{};

// Console + file (auto-creates logs/ dir with timestamped file + latest.log symlink)
dlog::Logger logger{"logs"};
```

### Example:
```cpp
dlog::Logger logger{std::filesystem::current_path()};

logger.log("Log message");
logger.debug("Debug message");
logger.warn("Warning message");
logger.error("FIRE AAAAH");
```
output:

<img width="638" height="93" alt="image" src="https://github.com/user-attachments/assets/dbfb6a6a-e0a3-42f8-83fc-a7c6aae802c6" />


[!WARNING]
This library requires C++20 or later!
