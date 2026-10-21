#ifndef DBUTILS_H
#define DBUTILS_H

#include <pqxx/pqxx>
#include <pqxx/binarystring>
#include <vector>
#include <cstdint>
#include <string>
#include <cstdio>

inline std::string vectorToByteaHex(const std::vector<uint8_t>& data) {
    if (data.empty()) return "''";
    std::string hex = "E'\\\\x";
    for (uint8_t c : data) {
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02x", c);
        hex += buf;
    }
    hex += "'";
    return hex;
}

inline std::vector<uint8_t> byteaToVector(pqxx::field field) {
    pqxx::binarystring bin(field);
    return std::vector<uint8_t>(bin.begin(), bin.end());
}

#endif