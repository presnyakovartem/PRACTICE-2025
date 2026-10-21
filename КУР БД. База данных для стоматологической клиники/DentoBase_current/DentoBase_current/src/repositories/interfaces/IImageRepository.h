#ifndef IIMAGEREPOSITORY_H
#define IIMAGEREPOSITORY_H

#include "domain/Image.h"

class IImageRepository {
public:
    virtual ~IImageRepository() = default;
    virtual void add(const Image& image) = 0;
    virtual Image getById(const uu_id& id) = 0;
};

#endif // IIMAGEREPOSITORY_H