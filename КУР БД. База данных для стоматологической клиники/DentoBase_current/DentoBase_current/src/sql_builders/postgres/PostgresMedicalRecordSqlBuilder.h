#ifndef POSTGRESMEDICALRECORDSQLBUILDER_H
#define POSTGRESMEDICALRECORDSQLBUILDER_H

#include "sql_builders/interfaces/IMedicalRecordSqlBuilder.h"

class PostgresMedicalRecordSqlBuilder : public IMedicalRecordSqlBuilder {
public:
    std::string buildSelectById(const uu_id& id) const override;
    std::string buildSelectActiveByPatientId(const uu_id& patientId) const override;
    std::string buildInsert(const MedicalRecord& record) const override;
    std::string buildUpdate(const MedicalRecord& record) const override;
};

#endif