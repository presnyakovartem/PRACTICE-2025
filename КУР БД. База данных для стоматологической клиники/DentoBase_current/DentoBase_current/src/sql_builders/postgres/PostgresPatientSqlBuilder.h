#ifndef POSTGRESPATIENTSQLBUILDER_H
#define POSTGRESPATIENTSQLBUILDER_H

#include "sql_builders/interfaces/IPatientSqlBuilder.h"

class PostgresPatientSqlBuilder : public IPatientSqlBuilder {
public:
    std::string buildSelectById(const uu_id& id) const override;
    std::string buildInsert(const Patient& patient) const override;
};

#endif