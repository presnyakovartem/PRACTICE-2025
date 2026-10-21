#ifndef DOCTOR_H
#define DOCTOR_H

#include <vector>
#include "Uuid.h"

enum class DoctorSpecialty {
    THERAPIST,
    ORTHODONTIST,
    SURGEON,
    PERIODONTIST,
    ENDODONTIST,
    IMPLANTOLOGIST,
    PEDIATRIC_DENTIST
};

struct Doctor {
    uu_id id;
    DoctorSpecialty specialty;

    // Связь M:N с клиниками хранится в таблице doctor_clinics.
    // В доменной модели врач содержит список связанных клиник.
    std::vector<uu_id> clinicIds;
};

#endif // DOCTOR_H
