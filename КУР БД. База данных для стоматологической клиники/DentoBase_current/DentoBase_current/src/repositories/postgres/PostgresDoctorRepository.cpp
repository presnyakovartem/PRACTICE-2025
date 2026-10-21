#include "PostgresDoctorRepository.h"
#include <stdexcept>

PostgresDoctorRepository::PostgresDoctorRepository(std::shared_ptr<pqxx::connection> conn,
                                                   std::shared_ptr<IDoctorSqlBuilder> sqlBuilder)
    : m_conn(conn), m_sqlBuilder(sqlBuilder) {}

static DoctorSpecialty stringToSpecialty(const std::string& spec) {
    if (spec == "THERAPIST") return DoctorSpecialty::THERAPIST;
    if (spec == "ORTHODONTIST") return DoctorSpecialty::ORTHODONTIST;
    if (spec == "SURGEON") return DoctorSpecialty::SURGEON;
    if (spec == "PERIODONTIST") return DoctorSpecialty::PERIODONTIST;
    if (spec == "ENDODONTIST") return DoctorSpecialty::ENDODONTIST;
    if (spec == "IMPLANTOLOGIST") return DoctorSpecialty::IMPLANTOLOGIST;
    if (spec == "PEDIATRIC_DENTIST") return DoctorSpecialty::PEDIATRIC_DENTIST;
    return DoctorSpecialty::THERAPIST;
}

Doctor PostgresDoctorRepository::getById(const uu_id& id) {
    if (id.isNull()) throw std::invalid_argument("Doctor ID is null");

    pqxx::work txn(*m_conn);
    auto res = txn.exec(m_sqlBuilder->buildSelectById(id));
    if (res.empty()) throw std::runtime_error("Doctor not found");

    Doctor d;
    d.id.value = res[0][0].as<std::string>();
    d.specialty = stringToSpecialty(res[0][1].as<std::string>());

    auto clinics = txn.exec(m_sqlBuilder->buildSelectClinicIds(id));
    for (const auto& row : clinics) {
        uu_id clinicId;
        clinicId.value = row[0].as<std::string>();
        d.clinicIds.push_back(clinicId);
    }

    txn.commit();
    return d;
}

void PostgresDoctorRepository::save(const Doctor& doctor) {
    if (doctor.id.isNull()) throw std::invalid_argument("Doctor ID is null");

    pqxx::work txn(*m_conn);
    txn.exec(m_sqlBuilder->buildInsert(doctor));

    // Сохраняем M:N-связи в той же транзакции, чтобы врач и его клиники
    // не могли оказаться в несогласованном состоянии.
    txn.exec(m_sqlBuilder->buildDeleteClinicLinks(doctor.id));
    for (const auto& clinicId : doctor.clinicIds) {
        if (clinicId.isNull()) throw std::invalid_argument("Clinic ID is null");
        txn.exec(m_sqlBuilder->buildInsertClinicLink(doctor.id, clinicId));
    }

    txn.commit();
}

void PostgresDoctorRepository::addClinic(const uu_id& doctorId, const uu_id& clinicId) {
    if (doctorId.isNull()) throw std::invalid_argument("Doctor ID is null");
    if (clinicId.isNull()) throw std::invalid_argument("Clinic ID is null");

    pqxx::work txn(*m_conn);
    txn.exec(m_sqlBuilder->buildInsertClinicLink(doctorId, clinicId));
    txn.commit();
}

std::vector<uu_id> PostgresDoctorRepository::getClinicIds(const uu_id& doctorId) {
    if (doctorId.isNull()) throw std::invalid_argument("Doctor ID is null");

    pqxx::work txn(*m_conn);
    auto res = txn.exec(m_sqlBuilder->buildSelectClinicIds(doctorId));

    std::vector<uu_id> clinicIds;
    for (const auto& row : res) {
        uu_id clinicId;
        clinicId.value = row[0].as<std::string>();
        clinicIds.push_back(clinicId);
    }

    txn.commit();
    return clinicIds;
}
