#ifndef POSTGRESDIAGNOSISREPOSITORY_H
#define POSTGRESDIAGNOSISREPOSITORY_H

#include "repositories/interfaces/IDiagnosisRepository.h"
#include "sql_builders/interfaces/IDiagnosisSqlBuilder.h"
#include <pqxx/pqxx>
#include <memory>

class PostgresDiagnosisRepository : public IDiagnosisRepository {
public:
    PostgresDiagnosisRepository(std::shared_ptr<pqxx::connection> conn,
                                std::shared_ptr<IDiagnosisSqlBuilder> sqlBuilder);
    void add(const Diagnosis& diagnosis) override;
    Diagnosis getById(const uu_id& id) override;

private:
    std::shared_ptr<pqxx::connection> m_conn;
    std::shared_ptr<IDiagnosisSqlBuilder> m_sqlBuilder;
};

#endif