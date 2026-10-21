#include "cache/RedisCache.h"

#include <stdexcept>
#include <utility>

RedisCache::RedisCache(
    const std::string& host,
    int port,
    std::shared_ptr<Logger> logger
)
    : context(nullptr),
      logger(std::move(logger)) {
    context = redisConnect(host.c_str(), port);

    if (context == nullptr) {
        throw std::runtime_error("Cannot allocate Redis context");
    }

    if (context->err) {
        std::string error = context->errstr;
        redisFree(context);
        context = nullptr;
        throw std::runtime_error("Cannot connect to Redis: " + error);
    }

    auto* reply = static_cast<redisReply*>(redisCommand(context, "PING"));
    if (reply == nullptr) {
        throw std::runtime_error("Redis PING failed");
    }

    bool ok = reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "PONG";
    freeReplyObject(reply);

    if (!ok) {
        throw std::runtime_error("Redis PING returned unexpected response");
    }

    this->logger->info("Connected to Redis cache");
}

RedisCache::~RedisCache() {
    if (context != nullptr) {
        redisFree(context);
        context = nullptr;
    }
}

std::optional<std::string> RedisCache::get(const std::string& key) {
    ensureConnected();

    auto* reply = static_cast<redisReply*>(
        redisCommand(context, "GET %s", key.c_str())
    );

    if (reply == nullptr) {
        throw std::runtime_error("Redis GET failed for key: " + key);
    }

    if (reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        return std::nullopt;
    }

    if (reply->type != REDIS_REPLY_STRING) {
        freeReplyObject(reply);
        throw std::runtime_error("Redis GET returned unexpected type for key: " + key);
    }

    std::string value(reply->str, reply->len);
    freeReplyObject(reply);

    return value;
}

void RedisCache::set(
    const std::string& key,
    const std::string& value,
    int ttlSeconds
) {
    ensureConnected();

    if (ttlSeconds <= 0) {
        throw std::invalid_argument("Redis TTL must be positive");
    }

    auto* reply = static_cast<redisReply*>(
        redisCommand(
            context,
            "SETEX %s %d %b",
            key.c_str(),
            ttlSeconds,
            value.data(),
            value.size()
        )
    );

    if (reply == nullptr) {
        throw std::runtime_error("Redis SETEX failed for key: " + key);
    }

    bool ok = reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK";
    freeReplyObject(reply);

    if (!ok) {
        throw std::runtime_error("Redis SETEX returned unexpected response for key: " + key);
    }
}

void RedisCache::remove(const std::string& key) {
    ensureConnected();

    auto* reply = static_cast<redisReply*>(
        redisCommand(context, "DEL %s", key.c_str())
    );

    if (reply == nullptr) {
        throw std::runtime_error("Redis DEL failed for key: " + key);
    }

    freeReplyObject(reply);
}

void RedisCache::ensureConnected() const {
    if (context == nullptr || context->err) {
        throw std::runtime_error("Redis connection is not available");
    }
}
