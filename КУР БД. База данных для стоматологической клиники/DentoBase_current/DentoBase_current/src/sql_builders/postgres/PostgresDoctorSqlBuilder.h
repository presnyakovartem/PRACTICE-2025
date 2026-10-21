#ifndef POSTGRESDOCTORSQLBUILDER_H
#define POSTGRESDOCTORSQLBUILDER_H

#include "sql_builders/interfaces/IDoctorSqlBuilder.h"

class PostgresDoctorSqlBuilder : public IDoctorSqlBuilder {
public:
    std::string buildSelectById(const uu_id& id) const override;
    std::string buildInsert(const Doctor& doctor) const override;

    std::string buildDeleteClinicLinks(const uu_id& doctorId) const override;
    std::string buildInsertClinicLink(const uu_id& doctorId, const uu_id& clinicId) const override;
    std::string buildSelectClinicIds(const uu_id& doctorId) const override;
};

#endif // POSTGRESDOCTORSQLBUILDER_H
