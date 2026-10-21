#include "PostgresDoctorSqlBuilder.h"
#include <sstream>

static std::string specialtyToString(DoctorSpecialty spec) {
    switch (spec) {
        case DoctorSpecialty::THERAPIST: return "THERAPIST";
        case DoctorSpecialty::ORTHODONTIST: return "ORTHODONTIST";
        case DoctorSpecialty::SURGEON: return "SURGEON";
        case DoctorSpecialty::PERIODONTIST: return "PERIODONTIST";
        case DoctorSpecialty::ENDODONTIST: return "ENDODONTIST";
        case DoctorSpecialty::IMPLANTOLOGIST: return "IMPLANTOLOGIST";
        case DoctorSpecialty::PEDIATRIC_DENTIST: return "PEDIATRIC_DENTIST";
        default: return "THERAPIST";
    }
}

std::string PostgresDoctorSqlBuilder::buildSelectById(const uu_id& id) const {
    std::stringstream sql;
    sql << "SELECT id, specialty FROM doctors WHERE id = '" << id.value << "'";
    return sql.str();
}

std::string PostgresDoctorSqlBuilder::buildInsert(const Doctor& doctor) const {
    std::stringstream sql;
    sql << "INSERT INTO doctors (id, specialty) VALUES ('"
        << doctor.id.value << "', '"
        << specialtyToString(doctor.specialty) << "') "
        << "ON CONFLICT (id) DO UPDATE SET specialty = EXCLUDED.specialty";
    return sql.str();
}

std::string PostgresDoctorSqlBuilder::buildDeleteClinicLinks(const uu_id& doctorId) const {
    std::stringstream sql;
    sql << "DELETE FROM doctor_clinics WHERE doctor_id = '" << doctorId.value << "'";
    return sql.str();
}

std::string PostgresDoctorSqlBuilder::buildInsertClinicLink(const uu_id& doctorId, const uu_id& clinicId) const {
    std::stringstream sql;
    sql << "INSERT INTO doctor_clinics (doctor_id, clinic_id) VALUES ('"
        << doctorId.value << "', '" << clinicId.value << "') "
        << "ON CONFLICT (doctor_id, clinic_id) DO NOTHING";
    return sql.str();
}

std::string PostgresDoctorSqlBuilder::buildSelectClinicIds(const uu_id& doctorId) const {
    std::stringstream sql;
    sql << "SELECT clinic_id FROM doctor_clinics WHERE doctor_id = '"
        << doctorId.value << "' ORDER BY clinic_id";
    return sql.str();
}
