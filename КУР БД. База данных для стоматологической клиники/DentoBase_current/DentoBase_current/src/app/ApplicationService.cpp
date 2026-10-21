#include "app/ApplicationService.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

ApplicationService::ApplicationService(
    std::shared_ptr<pqxx::connection> conn,
    std::shared_ptr<IPatientRepository> patientRepo,
    std::shared_ptr<IDoctorRepository> doctorRepo,
    std::shared_ptr<IMedicalRecordRepository> medicalRecordRepo,
    std::shared_ptr<IImageRepository> imageRepo,
    std::shared_ptr<IDiagnosisRepository> diagnosisRepo,
    std::shared_ptr<ImageService> imageService,
    std::shared_ptr<DiagnosisService> diagnosisService,
    std::shared_ptr<Logger> logger,
    int maxDiagnosisTextLength,
    std::shared_ptr<ICache> cache,
    int cacheTtlSeconds
)
    : conn(std::move(conn)),
      patientRepo(std::move(patientRepo)),
      doctorRepo(std::move(doctorRepo)),
      medicalRecordRepo(std::move(medicalRecordRepo)),
      imageRepo(std::move(imageRepo)),
      diagnosisRepo(std::move(diagnosisRepo)),
      imageService(std::move(imageService)),
      diagnosisService(std::move(diagnosisService)),
      logger(std::move(logger)),
      maxDiagnosisTextLength(maxDiagnosisTextLength),
      cache(std::move(cache)),
      cacheTtlSeconds(cacheTtlSeconds) {}

Patient ApplicationService::createPatient(const std::string& fullName, const std::tm& birthDate) {
    try {
        logger->info("User action: create patient");

        if (fullName.empty()) {
            throw std::invalid_argument("Patient full name is empty");
        }

        Patient patient;
        patient.id = uu_id::generate();
        patient.fullName = fullName;
        patient.birthDate = birthDate;

        patientRepo->save(patient);

        if (cache) {
            try {
                cache->set(patientCacheKey(patient.id), serializePatient(patient), cacheTtlSeconds);
                logger->info("Patient cached in Redis after create: " + patient.id.value);
            } catch (const std::exception& e) {
                logger->warning(std::string("Redis SET failed after patient create: ") + e.what());
            }
        }

        logger->info("Patient created: " + patient.id.value);
        return patient;
    } catch (const std::exception& e) {
        logger->error(std::string("createPatient failed: ") + e.what());
        throw;
    }
}

Patient ApplicationService::findPatient(const uu_id& patientId) {
    try {
        logger->info("User action: find patient " + patientId.value);

        if (cache) {
            const std::string key = patientCacheKey(patientId);
            try {
                auto cachedValue = cache->get(key);
                if (cachedValue.has_value()) {
                    logger->info("Redis cache hit: " + key);
                    return deserializePatient(cachedValue.value());
                }
                logger->info("Redis cache miss: " + key);
            } catch (const std::exception& e) {
                logger->warning(std::string("Redis GET failed, fallback to PostgreSQL: ") + e.what());
            }
        }

        Patient patient = patientRepo->getById(patientId);

        if (cache) {
            const std::string key = patientCacheKey(patientId);
            try {
                cache->set(key, serializePatient(patient), cacheTtlSeconds);
                logger->info("Patient cached in Redis: " + key);
            } catch (const std::exception& e) {
                logger->warning(std::string("Redis SET failed: ") + e.what());
            }
        }

        return patient;
    } catch (const std::exception& e) {
        logger->error(std::string("findPatient failed: ") + e.what());
        throw;
    }
}

MedicalRecord ApplicationService::createMedicalRecord(const uu_id& patientId, const std::string& notes) {
    try {
        logger->info("User action: create medical record for patient " + patientId.value);

        patientRepo->getById(patientId);

        MedicalRecord record;
        record.id = uu_id::generate();
        record.patientId = patientId;
        record.createdAt = std::time(nullptr);
        record.updatedAt = record.createdAt;
        record.isActive = true;
        record.notes = notes;

        medicalRecordRepo->add(record);

        logger->info("Medical record created: " + record.id.value);
        return record;
    } catch (const std::exception& e) {
        logger->error(std::string("createMedicalRecord failed: ") + e.what());
        throw;
    }
}

MedicalRecord ApplicationService::getMedicalRecord(const uu_id& medicalRecordId) {
    try {
        logger->info("User action: get medical record " + medicalRecordId.value);
        return medicalRecordRepo->getById(medicalRecordId);
    } catch (const std::exception& e) {
        logger->error(std::string("getMedicalRecord failed: ") + e.what());
        throw;
    }
}

uu_id ApplicationService::createClinic(const std::string& address) {
    try {
        logger->info("User action: create clinic");

        if (address.empty()) {
            throw std::invalid_argument("Clinic address is empty");
        }

        uu_id clinicId = uu_id::generate();

        pqxx::work txn(*conn);
        txn.exec(
            "INSERT INTO clinics (id, address) VALUES ('"
            + clinicId.value + "', '"
            + escapeSqlString(address) + "')"
        );
        txn.commit();

        logger->info("Clinic created: " + clinicId.value);
        return clinicId;
    } catch (const std::exception& e) {
        logger->error(std::string("createClinic failed: ") + e.what());
        throw;
    }
}

Doctor ApplicationService::createDoctor(DoctorSpecialty specialty, const std::vector<uu_id>& clinicIds) {
    try {
        logger->info("User action: create doctor");

        if (clinicIds.empty()) {
            throw std::invalid_argument("Doctor must be linked with at least one clinic");
        }

        Doctor doctor;
        doctor.id = uu_id::generate();
        doctor.specialty = specialty;
        doctor.clinicIds = clinicIds;

        doctorRepo->save(doctor);

        logger->info("Doctor created: " + doctor.id.value);
        return doctor;
    } catch (const std::exception& e) {
        logger->error(std::string("createDoctor failed: ") + e.what());
        throw;
    }
}

Doctor ApplicationService::getDoctor(const uu_id& doctorId) {
    try {
        logger->info("User action: get doctor " + doctorId.value);
        return doctorRepo->getById(doctorId);
    } catch (const std::exception& e) {
        logger->error(std::string("getDoctor failed: ") + e.what());
        throw;
    }
}

uu_id ApplicationService::createTechnician(const std::string& fullName, const uu_id& clinicId) {
    try {
        logger->info("User action: create technician");

        if (fullName.empty()) {
            throw std::invalid_argument("Technician full name is empty");
        }

        if (clinicId.isNull()) {
            throw std::invalid_argument("Clinic ID is empty");
        }

        uu_id technicianId = uu_id::generate();

        pqxx::work txn(*conn);
        txn.exec(
            "INSERT INTO technicians (id, full_name, clinic_id) VALUES ('"
            + technicianId.value + "', '"
            + escapeSqlString(fullName) + "', '"
            + clinicId.value + "')"
        );
        txn.commit();

        logger->info("Technician created: " + technicianId.value);
        return technicianId;
    } catch (const std::exception& e) {
        logger->error(std::string("createTechnician failed: ") + e.what());
        throw;
    }
}

uu_id ApplicationService::createEquipment(const std::string& model, const uu_id& clinicId) {
    try {
        logger->info("User action: create equipment");

        if (model.empty()) {
            throw std::invalid_argument("Equipment model is empty");
        }

        if (clinicId.isNull()) {
            throw std::invalid_argument("Clinic ID is empty");
        }

        uu_id equipmentId = uu_id::generate();

        pqxx::work txn(*conn);
        txn.exec(
            "INSERT INTO equipment (id, model, clinic_id) VALUES ('"
            + equipmentId.value + "', '"
            + escapeSqlString(model) + "', '"
            + clinicId.value + "')"
        );
        txn.commit();

        logger->info("Equipment created: " + equipmentId.value);
        return equipmentId;
    } catch (const std::exception& e) {
        logger->error(std::string("createEquipment failed: ") + e.what());
        throw;
    }
}

uu_id ApplicationService::createImageType(const std::string& name) {
    try {
        logger->info("User action: create image type");

        if (name.empty()) {
            throw std::invalid_argument("Image type name is empty");
        }

        uu_id typeId = uu_id::generate();

        pqxx::work txn(*conn);
        txn.exec(
            "INSERT INTO image_types (id, name) VALUES ('"
            + typeId.value + "', '"
            + escapeSqlString(name) + "')"
        );
        txn.commit();

        logger->info("Image type created: " + typeId.value);
        return typeId;
    } catch (const std::exception& e) {
        logger->error(std::string("createImageType failed: ") + e.what());
        throw;
    }
}

Image ApplicationService::uploadImage(
    const std::vector<uint8_t>& imageData,
    const uu_id& patientId,
    const uu_id& techId,
    const uu_id& equipId,
    const uu_id& typeId
) {
    try {
        logger->info("User action: upload image for patient " + patientId.value);

        Image image = imageService->uploadImage(imageData, patientId, techId, equipId, typeId);

        logger->info("Image uploaded: " + image.id.value);
        return image;
    } catch (const std::exception& e) {
        logger->error(std::string("uploadImage failed: ") + e.what());
        throw;
    }
}

Image ApplicationService::getImage(const uu_id& imageId) {
    try {
        logger->info("User action: get image " + imageId.value);
        return imageRepo->getById(imageId);
    } catch (const std::exception& e) {
        logger->error(std::string("getImage failed: ") + e.what());
        throw;
    }
}

Diagnosis ApplicationService::createDiagnosis(
    const std::string& text,
    const uu_id& imageId,
    const uu_id& doctorId
) {
    try {
        logger->info("User action: create diagnosis for image " + imageId.value);

        if (text.empty()) {
            throw std::invalid_argument("Diagnosis text is empty");
        }

        if (static_cast<int>(text.size()) > maxDiagnosisTextLength) {
            throw std::invalid_argument("Diagnosis text is too long");
        }

        Diagnosis diagnosis = diagnosisService->createDiagnosis(text, imageId, doctorId);

        logger->info("Diagnosis created: " + diagnosis.id.value);
        return diagnosis;
    } catch (const std::exception& e) {
        logger->error(std::string("createDiagnosis failed: ") + e.what());
        throw;
    }
}

Diagnosis ApplicationService::getDiagnosis(const uu_id& diagnosisId) {
    try {
        logger->info("User action: get diagnosis " + diagnosisId.value);
        return diagnosisRepo->getById(diagnosisId);
    } catch (const std::exception& e) {
        logger->error(std::string("getDiagnosis failed: ") + e.what());
        throw;
    }
}

void ApplicationService::showDictionaries() {
    try {
        logger->info("User action: show dictionaries");

        pqxx::work txn(*conn);

        std::cout << "\n=== Clinics ===\n";
        auto clinics = txn.exec("SELECT id, address FROM clinics ORDER BY address");
        for (const auto& row : clinics) {
            std::cout << row[0].as<std::string>() << " | " << row[1].as<std::string>() << "\n";
        }

        std::cout << "\n=== Doctors ===\n";
        auto doctors = txn.exec("SELECT id, specialty FROM doctors ORDER BY id");
        for (const auto& row : doctors) {
            std::cout << row[0].as<std::string>() << " | " << row[1].as<std::string>() << "\n";
        }

        std::cout << "\n=== Doctor clinics ===\n";
        auto doctorClinics = txn.exec(
            "SELECT dc.doctor_id, d.specialty, dc.clinic_id, c.address "
            "FROM doctor_clinics dc "
            "JOIN doctors d ON d.id = dc.doctor_id "
            "JOIN clinics c ON c.id = dc.clinic_id "
            "ORDER BY dc.doctor_id"
        );
        for (const auto& row : doctorClinics) {
            std::cout << "doctor=" << row[0].as<std::string>()
                      << " | specialty=" << row[1].as<std::string>()
                      << " | clinic=" << row[2].as<std::string>()
                      << " | address=" << row[3].as<std::string>() << "\n";
        }

        std::cout << "\n=== Technicians ===\n";
        auto technicians = txn.exec(
            "SELECT t.id, t.full_name, t.clinic_id, c.address "
            "FROM technicians t "
            "JOIN clinics c ON c.id = t.clinic_id "
            "ORDER BY t.full_name"
        );
        for (const auto& row : technicians) {
            std::cout << row[0].as<std::string>() << " | "
                      << row[1].as<std::string>() << " | clinic="
                      << row[2].as<std::string>() << " | "
                      << row[3].as<std::string>() << "\n";
        }

        std::cout << "\n=== Patients and medical records ===\n";
        auto patientsAndRecords = txn.exec(
            "SELECT "
            "p.id AS patient_id, "
            "p.full_name, "
            "p.birth_date, "
            "mr.id AS record_id, "
            "mr.created_at, "
            "mr.updated_at, "
            "mr.is_active, "
            "COALESCE(mr.notes, '') AS notes "
            "FROM patients p "
            "LEFT JOIN medical_records mr ON mr.patient_id = p.id "
            "ORDER BY p.full_name, mr.created_at DESC"
        );

        if (patientsAndRecords.empty()) {
            std::cout << "No patients.\n";
        } else {
            std::string currentPatientId;
            for (const auto& row : patientsAndRecords) {
                std::string patientId = row[0].as<std::string>();
                if (patientId != currentPatientId) {
                    currentPatientId = patientId;
                    std::cout << "\nPatient:\n";
                    std::cout << "  id: " << row[0].as<std::string>() << "\n";
                    std::cout << "  full_name: " << row[1].as<std::string>() << "\n";
                    std::cout << "  birth_date: " << row[2].as<std::string>() << "\n";
                    std::cout << "  medical_records:\n";
                }

                if (row[3].is_null()) {
                    std::cout << "    no medical records\n";
                } else {
                    std::cout << "    record_id: " << row[3].as<std::string>() << "\n";
                    std::cout << "      created_at: " << row[4].as<std::string>() << "\n";
                    std::cout << "      updated_at: " << row[5].as<std::string>() << "\n";
                    std::cout << "      is_active: " << (row[6].as<bool>() ? "true" : "false") << "\n";
                    std::cout << "      notes: " << row[7].as<std::string>() << "\n";
                }
            }
        }

        std::cout << "\n=== Equipment ===\n";
        auto equipment = txn.exec(
            "SELECT e.id, e.model, e.clinic_id, c.address "
            "FROM equipment e "
            "JOIN clinics c ON e.clinic_id = c.id "
            "ORDER BY e.model"
        );
        for (const auto& row : equipment) {
            std::cout << row[0].as<std::string>() << " | "
                      << row[1].as<std::string>() << " | clinic="
                      << row[2].as<std::string>() << " | "
                      << row[3].as<std::string>() << "\n";
        }

        std::cout << "\n=== Image types ===\n";
        auto imageTypes = txn.exec("SELECT id, name FROM image_types ORDER BY name");
        for (const auto& row : imageTypes) {
            std::cout << row[0].as<std::string>() << " | " << row[1].as<std::string>() << "\n";
        }

        txn.commit();
        logger->info("Dictionaries shown");
    } catch (const std::exception& e) {
        logger->error(std::string("showDictionaries failed: ") + e.what());
        throw;
    }
}

std::string ApplicationService::escapeSqlString(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }
    return result;
}

std::string ApplicationService::patientCacheKey(const uu_id& patientId) {
    return "patient:" + patientId.value;
}

std::string ApplicationService::serializePatient(const Patient& patient) {
    return patient.id.value + "\n" + patient.fullName + "\n" + dateToString(patient.birthDate);
}

Patient ApplicationService::deserializePatient(const std::string& value) {
    std::size_t first = value.find('\n');
    std::size_t second = value.find('\n', first + 1);

    if (first == std::string::npos || second == std::string::npos) {
        throw std::runtime_error("Invalid cached patient format");
    }

    Patient patient;
    patient.id.value = value.substr(0, first);
    patient.fullName = value.substr(first + 1, second - first - 1);
    patient.birthDate = stringToDate(value.substr(second + 1));

    return patient;
}

std::string ApplicationService::dateToString(const std::tm& date) {
    std::ostringstream ss;
    ss << std::put_time(&date, "%Y-%m-%d");
    return ss.str();
}

std::tm ApplicationService::stringToDate(const std::string& value) {
    std::tm date{};
    std::istringstream ss(value);
    ss >> std::get_time(&date, "%Y-%m-%d");

    if (ss.fail()) {
        throw std::runtime_error("Invalid cached patient date");
    }

    return date;
}

TableData ApplicationService::executeTableQuery(
    const std::vector<std::string>& headers,
    const std::string& sql
) {
    pqxx::work txn(*conn);

    TableData table;
    table.headers = headers;

    auto result = txn.exec(sql);

    for (const auto& row : result) {
        std::vector<std::string> values;

        for (std::size_t i = 0; i < row.size(); ++i) {
            if (row[i].is_null()) {
                values.emplace_back("");
            } else {
                values.emplace_back(row[i].as<std::string>());
            }
        }

        table.rows.push_back(values);
    }

    txn.commit();
    return table;
}

void ApplicationService::executeDeleteById(
    const std::string& tableName,
    const uu_id& id,
    const std::string& entityName
) {
    try {
        logger->info("User action: delete " + entityName + " " + id.value);

        pqxx::work txn(*conn);

        auto result = txn.exec(
            "DELETE FROM " + tableName + " WHERE id = " + txn.quote(id.value)
        );

        txn.commit();

        if (result.affected_rows() == 0) {
            throw std::runtime_error(entityName + " not found: " + id.value);
        }

        logger->info(entityName + " deleted: " + id.value);
    } catch (const std::exception& e) {
        logger->error("delete " + entityName + " failed: " + std::string(e.what()));
        throw;
    }
}

void ApplicationService::deleteClinic(const uu_id& clinicId) {
    executeDeleteById("clinics", clinicId, "clinic");
}

void ApplicationService::deleteDoctor(const uu_id& doctorId) {
    executeDeleteById("doctors", doctorId, "doctor");
}

void ApplicationService::deleteTechnician(const uu_id& technicianId) {
    executeDeleteById("technicians", technicianId, "technician");
}

void ApplicationService::deleteEquipment(const uu_id& equipmentId) {
    executeDeleteById("equipment", equipmentId, "equipment");
}

void ApplicationService::deleteImageType(const uu_id& typeId) {
    executeDeleteById("image_types", typeId, "image_type");
}

void ApplicationService::deleteDiagnosis(const uu_id& diagnosisId) {
    executeDeleteById("diagnoses", diagnosisId, "diagnosis");
}

TableData ApplicationService::getClinicsTable() {
    return executeTableQuery(
        {"ID", "Адрес"},
        "SELECT id::text, address FROM clinics ORDER BY address"
    );
}

TableData ApplicationService::getClinicTable(const uu_id& clinicId) {
    pqxx::work txn(*conn);
    std::string sql =
        "SELECT id::text, address FROM clinics WHERE id = "
        + txn.quote(clinicId.value);
    txn.commit();

    return executeTableQuery({"ID", "Адрес"}, sql);
}

TableData ApplicationService::getDoctorsTable() {
    return executeTableQuery(
        {"ID", "Специальность", "Клиники"},
        "SELECT "
        "d.id::text, "
        "d.specialty, "
        "COALESCE(string_agg(c.address, ', '), '') AS clinics "
        "FROM doctors d "
        "LEFT JOIN doctor_clinics dc ON dc.doctor_id = d.id "
        "LEFT JOIN clinics c ON c.id = dc.clinic_id "
        "GROUP BY d.id, d.specialty "
        "ORDER BY d.id"
    );
}

TableData ApplicationService::getDoctorTable(const uu_id& doctorId) {
    pqxx::work txn(*conn);
    std::string sql =
        "SELECT "
        "d.id::text, "
        "d.specialty, "
        "COALESCE(string_agg(c.address, ', '), '') AS clinics "
        "FROM doctors d "
        "LEFT JOIN doctor_clinics dc ON dc.doctor_id = d.id "
        "LEFT JOIN clinics c ON c.id = dc.clinic_id "
        "WHERE d.id = " + txn.quote(doctorId.value) + " "
        "GROUP BY d.id, d.specialty";
    txn.commit();

    return executeTableQuery({"ID", "Специальность", "Клиники"}, sql);
}

TableData ApplicationService::getTechniciansTable() {
    return executeTableQuery(
        {"ID", "ФИО", "ID клиники", "Клиника"},
        "SELECT "
        "t.id::text, "
        "t.full_name, "
        "t.clinic_id::text, "
        "c.address "
        "FROM technicians t "
        "JOIN clinics c ON c.id = t.clinic_id "
        "ORDER BY t.full_name"
    );
}

TableData ApplicationService::getTechnicianTable(const uu_id& technicianId) {
    pqxx::work txn(*conn);
    std::string sql =
        "SELECT "
        "t.id::text, "
        "t.full_name, "
        "t.clinic_id::text, "
        "c.address "
        "FROM technicians t "
        "JOIN clinics c ON c.id = t.clinic_id "
        "WHERE t.id = " + txn.quote(technicianId.value);
    txn.commit();

    return executeTableQuery({"ID", "ФИО", "ID клиники", "Клиника"}, sql);
}

TableData ApplicationService::getEquipmentTable() {
    return executeTableQuery(
        {"ID", "Модель", "ID клиники", "Клиника"},
        "SELECT "
        "e.id::text, "
        "e.model, "
        "e.clinic_id::text, "
        "c.address "
        "FROM equipment e "
        "JOIN clinics c ON c.id = e.clinic_id "
        "ORDER BY e.model"
    );
}

TableData ApplicationService::getEquipmentItemTable(const uu_id& equipmentId) {
    pqxx::work txn(*conn);
    std::string sql =
        "SELECT "
        "e.id::text, "
        "e.model, "
        "e.clinic_id::text, "
        "c.address "
        "FROM equipment e "
        "JOIN clinics c ON c.id = e.clinic_id "
        "WHERE e.id = " + txn.quote(equipmentId.value);
    txn.commit();

    return executeTableQuery({"ID", "Модель", "ID клиники", "Клиника"}, sql);
}

TableData ApplicationService::getImageTypesTable() {
    return executeTableQuery(
        {"ID", "Название"},
        "SELECT id::text, name FROM image_types ORDER BY name"
    );
}

TableData ApplicationService::getImageTypeTable(const uu_id& typeId) {
    pqxx::work txn(*conn);
    std::string sql =
        "SELECT id::text, name FROM image_types WHERE id = "
        + txn.quote(typeId.value);
    txn.commit();

    return executeTableQuery({"ID", "Название"}, sql);
}

TableData ApplicationService::getPatientsWithRecordsTable() {
    return executeTableQuery(
        {
            "ID пациента",
            "ФИО",
            "Дата рождения",
            "ID медкарты",
            "Создана",
            "Обновлена",
            "Активна",
            "Примечания"
        },
        "SELECT "
        "p.id::text, "
        "p.full_name, "
        "p.birth_date::text, "
        "COALESCE(mr.id::text, ''), "
        "COALESCE(mr.created_at::text, ''), "
        "COALESCE(mr.updated_at::text, ''), "
        "COALESCE(mr.is_active::text, ''), "
        "COALESCE(mr.notes, '') "
        "FROM patients p "
        "LEFT JOIN medical_records mr ON mr.patient_id = p.id "
        "ORDER BY p.full_name, mr.created_at DESC"
    );
}

TableData ApplicationService::getDiagnosesTable() {
    return executeTableQuery(
        {"ID", "Текст", "Дата", "ID снимка", "ID врача", "ID медкарты"},
        "SELECT "
        "id::text, "
        "text, "
        "created_at::text, "
        "image_id::text, "
        "doctor_id::text, "
        "medical_record_id::text "
        "FROM diagnoses "
        "ORDER BY created_at DESC"
    );
}

TableData ApplicationService::getDiagnosisTable(const uu_id& diagnosisId) {
    pqxx::work txn(*conn);
    std::string sql =
        "SELECT "
        "id::text, "
        "text, "
        "created_at::text, "
        "image_id::text, "
        "doctor_id::text, "
        "medical_record_id::text "
        "FROM diagnoses "
        "WHERE id = " + txn.quote(diagnosisId.value);
    txn.commit();

    return executeTableQuery(
        {"ID", "Текст", "Дата", "ID снимка", "ID врача", "ID медкарты"},
        sql
    );
}