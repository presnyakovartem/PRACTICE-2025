#include "PostgresPatientRepository.h"
#include <stdexcept>
#include <iomanip>

PostgresPatientRepository::PostgresPatientRepository(std::shared_ptr<pqxx::connection> conn,
                                                     std::shared_ptr<IPatientSqlBuilder> sqlBuilder)
    : m_conn(conn), m_sqlBuilder(sqlBuilder) {}

Patient PostgresPatientRepository::getById(const uu_id& id) {
    if (id.isNull()) throw std::invalid_argument("Patient ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildSelectById(id);
    auto res = txn.exec(sql);
    if (res.empty()) throw std::runtime_error("Patient not found");
    Patient p;
    p.id.value = res[0][0].as<std::string>();
    p.fullName = res[0][1].as<std::string>();
    std::string dateStr = res[0][2].as<std::string>();
    std::istringstream ss(dateStr);
    ss >> std::get_time(&p.birthDate, "%Y-%m-%d");
    txn.commit();
    return p;
}

void PostgresPatientRepository::save(const Patient& patient) {
    if (patient.id.isNull()) throw std::invalid_argument("Patient ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildInsert(patient);
    txn.exec(sql);
    txn.commit();
}