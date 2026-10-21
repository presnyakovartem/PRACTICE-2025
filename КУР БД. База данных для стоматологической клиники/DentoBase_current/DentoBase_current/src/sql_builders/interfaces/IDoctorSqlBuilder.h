#ifndef IDOCTORSQLBUILDER_H
#define IDOCTORSQLBUILDER_H

#include <string>
#include "domain/Doctor.h"

class IDoctorSqlBuilder {
public:
    virtual ~IDoctorSqlBuilder() = default;
    virtual std::string buildSelectById(const uu_id& id) const = 0;
    virtual std::string buildInsert(const Doctor& doctor) const = 0;

    virtual std::string buildDeleteClinicLinks(const uu_id& doctorId) const = 0;
    virtual std::string buildInsertClinicLink(const uu_id& doctorId, const uu_id& clinicId) const = 0;
    virtual std::string buildSelectClinicIds(const uu_id& doctorId) const = 0;
};

#endif // IDOCTORSQLBUILDER_H
