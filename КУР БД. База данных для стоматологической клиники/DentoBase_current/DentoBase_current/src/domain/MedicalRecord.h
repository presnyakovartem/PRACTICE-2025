#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H

#include <string>
#include <ctime>
#include "Uuid.h"

struct MedicalRecord {
    uu_id id;
    uu_id patientId;
    std::time_t createdAt;
    std::time_t updatedAt;
    bool isActive;
    std::string notes;
};

#endif // MEDICALRECORD_H