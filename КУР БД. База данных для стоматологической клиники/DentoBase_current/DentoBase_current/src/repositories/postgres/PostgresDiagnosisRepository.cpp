#include "PostgresDiagnosisRepository.h"
#include <stdexcept>

PostgresDiagnosisRepository::PostgresDiagnosisRepository(std::shared_ptr<pqxx::connection> conn,
                                                         std::shared_ptr<IDiagnosisSqlBuilder> sqlBuilder)
    : m_conn(conn), m_sqlBuilder(sqlBuilder) {}

void PostgresDiagnosisRepository::add(const Diagnosis& diagnosis) {
    if (diagnosis.id.isNull()) throw std::invalid_argument("Diagnosis ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildInsert(diagnosis);
    txn.exec(sql);
    txn.commit();
}

Diagnosis PostgresDiagnosisRepository::getById(const uu_id& id) {
    if (id.isNull()) throw std::invalid_argument("Diagnosis ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildSelectById(id);
    auto res = txn.exec(sql);
    if (res.empty()) throw std::runtime_error("Diagnosis not found");
    Diagnosis d;
    d.id.value = res[0][0].as<std::string>();
    d.text = res[0][1].as<std::string>();
    d.createdAt = static_cast<std::time_t>(res[0][2].as<double>());
    d.imageId.value = res[0][3].as<std::string>();
    d.doctorId.value = res[0][4].as<std::string>();
    d.medicalRecordId.value = res[0][5].as<std::string>();
    txn.commit();
    return d;
}