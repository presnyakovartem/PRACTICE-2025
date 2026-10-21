#ifndef IMEDICALRECORDSQLBUILDER_H
#define IMEDICALRECORDSQLBUILDER_H

#include <string>
#include "domain/MedicalRecord.h"

class IMedicalRecordSqlBuilder {
public:
    virtual ~IMedicalRecordSqlBuilder() = default;
    virtual std::string buildSelectById(const uu_id& id) const = 0;
    virtual std::string buildSelectActiveByPatientId(const uu_id& patientId) const = 0;
    virtual std::string buildInsert(const MedicalRecord& record) const = 0;
    virtual std::string buildUpdate(const MedicalRecord& record) const = 0;
};

#endif