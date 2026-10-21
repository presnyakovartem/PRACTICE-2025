#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <vector>
#include <ctime>
#include "Uuid.h"

struct Image {
    uu_id id;
    std::vector<uint8_t> imageData;
    std::time_t uploadDate;
    uu_id patientId;
    uu_id techId;
    uu_id equipId;
    uu_id typeId;
};

#endif // IMAGE_H