#ifndef IPATIENTREPOSITORY_H
#define IPATIENTREPOSITORY_H

#include "domain/Patient.h"

class IPatientRepository {
public:
    virtual ~IPatientRepository() = default;
    virtual Patient getById(const uu_id& id) = 0;
    virtual void save(const Patient& patient) = 0;
};

#endif // IPATIENTREPOSITORY_H