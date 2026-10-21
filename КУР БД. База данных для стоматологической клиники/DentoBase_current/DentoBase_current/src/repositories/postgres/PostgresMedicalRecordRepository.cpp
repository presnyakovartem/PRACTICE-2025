#include "PostgresMedicalRecordRepository.h"
#include <stdexcept>

PostgresMedicalRecordRepository::PostgresMedicalRecordRepository(std::shared_ptr<pqxx::connection> conn,
                                                                 std::shared_ptr<IMedicalRecordSqlBuilder> sqlBuilder)
    : m_conn(conn), m_sqlBuilder(sqlBuilder) {}

MedicalRecord PostgresMedicalRecordRepository::getById(const uu_id& id) {
    if (id.isNull()) throw std::invalid_argument("MedicalRecord ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildSelectById(id);
    auto res = txn.exec(sql);
    if (res.empty()) throw std::runtime_error("MedicalRecord not found");
    MedicalRecord r;
    r.id.value = res[0][0].as<std::string>();
    r.patientId.value = res[0][1].as<std::string>();
    r.createdAt = static_cast<std::time_t>(res[0][2].as<double>());
    r.updatedAt = static_cast<std::time_t>(res[0][3].as<double>());
    r.isActive = res[0][4].as<bool>();
    r.notes = res[0][5].as<std::string>();
    txn.commit();
    return r;
}

MedicalRecord PostgresMedicalRecordRepository::getActiveByPatientId(const uu_id& patientId) {
    if (patientId.isNull()) throw std::invalid_argument("Patient ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildSelectActiveByPatientId(patientId);
    auto res = txn.exec(sql);
    if (res.empty()) throw std::runtime_error("No active medical record for patient");
    MedicalRecord r;
    r.id.value = res[0][0].as<std::string>();
    r.patientId.value = res[0][1].as<std::string>();
    r.createdAt = static_cast<std::time_t>(res[0][2].as<double>());
    r.updatedAt = static_cast<std::time_t>(res[0][3].as<double>());
    r.isActive = res[0][4].as<bool>();
    r.notes = res[0][5].as<std::string>();
    txn.commit();
    return r;
}

void PostgresMedicalRecordRepository::add(const MedicalRecord& record) {
    if (record.id.isNull()) throw std::invalid_argument("MedicalRecord ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildInsert(record);
    txn.exec(sql);
    txn.commit();
}

void PostgresMedicalRecordRepository::update(const MedicalRecord& record) {
    if (record.id.isNull()) throw std::invalid_argument("MedicalRecord ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildUpdate(record);
    txn.exec(sql);
    txn.commit();
}