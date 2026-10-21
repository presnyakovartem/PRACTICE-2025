#ifndef IIMAGESQLBUILDER_H
#define IIMAGESQLBUILDER_H

#include <string>
#include "domain/Image.h"

class IImageSqlBuilder {
public:
    virtual ~IImageSqlBuilder() = default;
    virtual std::string buildSelectById(const uu_id& id) const = 0;
    virtual std::string buildInsert(const Image& image) const = 0;
};

#endif