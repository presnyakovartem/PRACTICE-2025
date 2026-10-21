#ifndef IDOCTORREPOSITORY_H
#define IDOCTORREPOSITORY_H

#include <vector>
#include "domain/Doctor.h"

class IDoctorRepository {
public:
    virtual ~IDoctorRepository() = default;
    virtual Doctor getById(const uu_id& id) = 0;
    virtual void save(const Doctor& doctor) = 0;

    // Работа со связью многие-ко-многим doctors <-> clinics.
    virtual void addClinic(const uu_id& doctorId, const uu_id& clinicId) = 0;
    virtual std::vector<uu_id> getClinicIds(const uu_id& doctorId) = 0;
};

#endif // IDOCTORREPOSITORY_H
