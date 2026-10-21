#include "PostgresMedicalRecordSqlBuilder.h"
#include <sstream>

std::string PostgresMedicalRecordSqlBuilder::buildSelectById(const uu_id& id) const {
    std::stringstream sql;
    sql << "SELECT id, patient_id, extract(epoch from created_at), extract(epoch from updated_at), is_active, notes "
        << "FROM medical_records WHERE id = '" << id.value << "'";
    return sql.str();
}

std::string PostgresMedicalRecordSqlBuilder::buildSelectActiveByPatientId(const uu_id& patientId) const {
    std::stringstream sql;
    sql << "SELECT id, patient_id, extract(epoch from created_at), extract(epoch from updated_at), is_active, notes "
        << "FROM medical_records WHERE patient_id = '" << patientId.value << "' AND is_active = true ORDER BY created_at DESC LIMIT 1";
    return sql.str();
}

std::string PostgresMedicalRecordSqlBuilder::buildInsert(const MedicalRecord& record) const {
    std::stringstream sql;
    sql << "INSERT INTO medical_records (id, patient_id, created_at, updated_at, is_active, notes) VALUES ('"
        << record.id.value << "', '"
        << record.patientId.value << "', to_timestamp(" << record.createdAt << "), to_timestamp(" << record.updatedAt << "), "
        << (record.isActive ? "true" : "false") << ", '"
        << record.notes << "')";
    return sql.str();
}

std::string PostgresMedicalRecordSqlBuilder::buildUpdate(const MedicalRecord& record) const {
    std::stringstream sql;
    sql << "UPDATE medical_records SET updated_at = to_timestamp(" << record.updatedAt << "), is_active = "
        << (record.isActive ? "true" : "false") << ", notes = '"
        << record.notes << "' WHERE id = '" << record.id.value << "'";
    return sql.str();
}