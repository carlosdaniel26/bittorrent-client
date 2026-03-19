#include "helpers.h"

std::string humanReadableSize(int64_t bytes) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    double size = static_cast<double>(bytes);
    int i = 0;
    while (size >= 1024 && i < 5) {
        size /= 1024;
        i++;
    }
    
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << size << " " << suffixes[i];
    return out.str();
}

std::string humanReadableTime(int64_t seconds) {
    std::time_t t = seconds;
    std::tm* tm_info = std::localtime(&t);

    std::ostringstream out;
    out << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
    return out.str();
}