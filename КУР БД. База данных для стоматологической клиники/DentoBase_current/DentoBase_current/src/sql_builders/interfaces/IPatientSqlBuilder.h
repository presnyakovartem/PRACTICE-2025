#ifndef IPATIENTSQLBUILDER_H
#define IPATIENTSQLBUILDER_H

#include <string>
#include "domain/Patient.h"

class IPatientSqlBuilder {
public:
    virtual ~IPatientSqlBuilder() = default;
    virtual std::string buildSelectById(const uu_id& id) const = 0;
    virtual std::string buildInsert(const Patient& patient) const = 0;
};

#endif