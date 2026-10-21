#include "PostgresPatientSqlBuilder.h"
#include <sstream>
#include <ctime>

std::string PostgresPatientSqlBuilder::buildSelectById(const uu_id& id) const {
    std::stringstream sql;
    sql << "SELECT id, full_name, birth_date FROM patients WHERE id = '" << id.value << "'";
    return sql.str();
}

std::string PostgresPatientSqlBuilder::buildInsert(const Patient& patient) const {
    char birthBuf[11];
    std::strftime(birthBuf, sizeof(birthBuf), "%Y-%m-%d", &patient.birthDate);
    std::stringstream sql;
    sql << "INSERT INTO patients (id, full_name, birth_date) VALUES ('"
        << patient.id.value << "', '"
        << patient.fullName << "', '"
        << birthBuf << "') ON CONFLICT (id) DO UPDATE SET full_name = EXCLUDED.full_name";
    return sql.str();
}