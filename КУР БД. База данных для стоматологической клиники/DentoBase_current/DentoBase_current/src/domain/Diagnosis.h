#ifndef DIAGNOSIS_H
#define DIAGNOSIS_H

#include <string>
#include <ctime>
#include "Uuid.h"

struct Diagnosis {
    uu_id id;
    std::string text;
    std::time_t createdAt;
    uu_id imageId;
    uu_id doctorId;
    uu_id medicalRecordId;
};

#endif // DIAGNOSIS_H