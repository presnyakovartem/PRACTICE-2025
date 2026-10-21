#ifndef ICACHE_H
#define ICACHE_H

#include <optional>
#include <string>

class ICache {
public:
    virtual ~ICache() = default;

    virtual std::optional<std::string> get(const std::string& key) = 0;

    virtual void set(
        const std::string& key,
        const std::string& value,
        int ttlSeconds
    ) = 0;

    virtual void remove(const std::string& key) = 0;
};

#endif // ICACHE_H
