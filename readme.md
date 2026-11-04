# Dumb Logger
Header only library to output logs to console and file.

> [!WARNING]
> This library requires C++20 or later!

## Usage
### Loggers
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

### Typed logger
```cpp
class Worker {
    dlog::TypedLogger<Worker> m_logger{};

public:
    void run() {
        m_logger.log("starting");
    }
};
```

The typed logger is a lightweight interface over `dlog::default_logger()`, shared
by all typed logger instances. It adds the type name to each message. By
default, it writes to the console at `Level::INFO`. To enable file output for
typed loggers, configure their shared logger before logging:

```cpp
dlog::set_default_log_directory("logs");
```

This creates the same timestamped log file and `latest.log` symlink as a
file-backed `Logger`.
