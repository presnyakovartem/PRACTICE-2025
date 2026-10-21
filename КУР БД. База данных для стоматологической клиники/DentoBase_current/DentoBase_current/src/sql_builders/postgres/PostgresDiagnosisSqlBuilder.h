#ifndef POSTGRESDIAGNOSISSQLBUILDER_H
#define POSTGRESDIAGNOSISSQLBUILDER_H

#include "sql_builders/interfaces/IDiagnosisSqlBuilder.h"

class PostgresDiagnosisSqlBuilder : public IDiagnosisSqlBuilder {
public:
    std::string buildSelectById(const uu_id& id) const override;
    std::string buildInsert(const Diagnosis& diagnosis) const override;
};

#endif