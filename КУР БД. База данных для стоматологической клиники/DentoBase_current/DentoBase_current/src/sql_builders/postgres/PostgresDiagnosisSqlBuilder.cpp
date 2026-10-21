#include "PostgresDiagnosisSqlBuilder.h"
#include <sstream>
#include <string>

std::string PostgresDiagnosisSqlBuilder::buildSelectById(const uu_id& id) const {
    std::stringstream sql;
    sql << "SELECT id, text, extract(epoch from created_at), image_id, doctor_id, medical_record_id "
        << "FROM diagnoses WHERE id = '" << id.value << "'";
    return sql.str();
}

static std::string escapeString(const std::string& s) {
    std::string escaped;
    for (char c : s) {
        if (c == '\'') escaped += "''";
        else escaped += c;
    }
    return escaped;
}

std::string PostgresDiagnosisSqlBuilder::buildInsert(const Diagnosis& diagnosis) const {
    std::stringstream sql;
    sql << "INSERT INTO diagnoses (id, text, created_at, image_id, doctor_id, medical_record_id) VALUES ('"
        << diagnosis.id.value << "', '"
        << escapeString(diagnosis.text) << "', "
        << "to_timestamp(" << diagnosis.createdAt << "), '"
        << diagnosis.imageId.value << "', '"
        << diagnosis.doctorId.value << "', '"
        << diagnosis.medicalRecordId.value << "')";
    return sql.str();
}