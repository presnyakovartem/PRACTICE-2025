#include "config/Config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>

Config::Config(const std::string& path) {
    load(path);
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = values.find(key);
    if (it == values.end()) {
        return defaultValue;
    }

    return it->second;
}

int Config::getInt(const std::string& key, int defaultValue) const {
    auto it = values.find(key);
    if (it == values.end()) {
        return defaultValue;
    }

    try {
        return std::stoi(it->second);
    } catch (...) {
        return defaultValue;
    }
}

void Config::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (!key.empty()) {
            values[key] = value;
        }
    }
}

std::string Config::trim(const std::string& value) {
    auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c);
    });

    auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) {
        return std::isspace(c);
    }).base();

    if (begin >= end) {
        return "";
    }

    return std::string(begin, end);
}
