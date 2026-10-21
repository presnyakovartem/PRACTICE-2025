#ifndef REDISCACHE_H
#define REDISCACHE_H

#include <memory>
#include <optional>
#include <string>

#include <hiredis/hiredis.h>

#include "cache/ICache.h"
#include "logging/Logger.h"

class RedisCache : public ICache {
public:
    RedisCache(
        const std::string& host,
        int port,
        std::shared_ptr<Logger> logger
    );

    ~RedisCache() override;

    std::optional<std::string> get(const std::string& key) override;

    void set(
        const std::string& key,
        const std::string& value,
        int ttlSeconds
    ) override;

    void remove(const std::string& key) override;

private:
    redisContext* context;
    std::shared_ptr<Logger> logger;

    void ensureConnected() const;
};

#endif // REDISCACHE_H
