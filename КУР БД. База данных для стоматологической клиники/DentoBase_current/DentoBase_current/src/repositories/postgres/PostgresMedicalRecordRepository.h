#ifndef POSTGRESMEDICALRECORDREPOSITORY_H
#define POSTGRESMEDICALRECORDREPOSITORY_H

#include "repositories/interfaces/IMedicalRecordRepository.h"
#include "sql_builders/interfaces/IMedicalRecordSqlBuilder.h"
#include <pqxx/pqxx>
#include <memory>

class PostgresMedicalRecordRepository : public IMedicalRecordRepository {
public:
    PostgresMedicalRecordRepository(std::shared_ptr<pqxx::connection> conn,
                                    std::shared_ptr<IMedicalRecordSqlBuilder> sqlBuilder);
    MedicalRecord getById(const uu_id& id) override;
    MedicalRecord getActiveByPatientId(const uu_id& patientId) override;
    void add(const MedicalRecord& record) override;
    void update(const MedicalRecord& record) override;

private:
    std::shared_ptr<pqxx::connection> m_conn;
    std::shared_ptr<IMedicalRecordSqlBuilder> m_sqlBuilder;
};

#endif