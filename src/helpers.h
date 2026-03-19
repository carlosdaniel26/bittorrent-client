#pragma once

#include <string>
#include <sstream>
#include <iomanip>

std::string humanReadableSize(int64_t bytes);
std::string humanReadableTime(int64_t seconds);