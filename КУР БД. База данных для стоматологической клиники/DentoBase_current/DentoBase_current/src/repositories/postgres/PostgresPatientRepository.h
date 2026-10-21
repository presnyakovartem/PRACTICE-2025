#ifndef POSTGRESPATIENTREPOSITORY_H
#define POSTGRESPATIENTREPOSITORY_H

#include "repositories/interfaces/IPatientRepository.h"
#include "sql_builders/interfaces/IPatientSqlBuilder.h"
#include <pqxx/pqxx>
#include <memory>

class PostgresPatientRepository : public IPatientRepository {
public:
    PostgresPatientRepository(std::shared_ptr<pqxx::connection> conn,
                              std::shared_ptr<IPatientSqlBuilder> sqlBuilder);
    Patient getById(const uu_id& id) override;
    void save(const Patient& patient) override;

private:
    std::shared_ptr<pqxx::connection> m_conn;
    std::shared_ptr<IPatientSqlBuilder> m_sqlBuilder;
};

#endif