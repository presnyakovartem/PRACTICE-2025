#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

class Config {
public:
    explicit Config(const std::string& path);

    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;

private:
    std::unordered_map<std::string, std::string> values;

    void load(const std::string& path);
    static std::string trim(const std::string& value);
};

#endif // CONFIG_H
