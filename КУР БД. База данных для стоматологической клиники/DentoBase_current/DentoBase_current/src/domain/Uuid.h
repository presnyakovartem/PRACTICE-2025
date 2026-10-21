#ifndef UUID_H
#define UUID_H

#include <string>
#include <random>
#include <sstream>
#include <iomanip>

struct uu_id {
    std::string value;
    
    static uu_id generate() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        std::uniform_int_distribution<> dis2(8, 11);
        
        std::stringstream ss;
        for (int i = 0; i < 36; ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                ss << '-';
            } else if (i == 14) {
                ss << '4';  // версия 4
            } else if (i == 19) {
                ss << std::hex << dis2(gen);
            } else {
                ss << std::hex << dis(gen);
            }
        }
        uu_id id;
        id.value = ss.str();
        return id;
    }
    
    bool operator==(const uu_id& other) const {
        return value == other.value;
    }
    
    bool operator!=(const uu_id& other) const {
        return !(*this == other);
    }
    
    bool isNull() const {
        return value.empty();
    }
};

#endif // UUID_H