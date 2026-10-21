#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include <ctime>
#include "Uuid.h"

struct Patient {
    uu_id id;
    std::string fullName;
    std::tm birthDate;
};

#endif // PATIENT_H