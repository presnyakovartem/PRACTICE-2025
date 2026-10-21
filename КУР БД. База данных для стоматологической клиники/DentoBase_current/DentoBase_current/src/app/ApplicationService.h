#ifndef APPLICATIONSERVICE_H
#define APPLICATIONSERVICE_H

#include <memory>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include "app/IApplicationService.h"
#include "cache/ICache.h"
#include "logging/Logger.h"

#include "repositories/interfaces/IPatientRepository.h"
#include "repositories/interfaces/IDoctorRepository.h"
#include "repositories/interfaces/IMedicalRecordRepository.h"
#include "repositories/interfaces/IImageRepository.h"
#include "repositories/interfaces/IDiagnosisRepository.h"

#include "services/ImageService.h"
#include "services/DiagnosisService.h"

class ApplicationService : public IApplicationService {
public:
    ApplicationService(
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
    );

    Patient createPatient(const std::string& fullName, const std::tm& birthDate) override;
    Patient findPatient(const uu_id& patientId) override;

    MedicalRecord createMedicalRecord(const uu_id& patientId, const std::string& notes) override;
    MedicalRecord getMedicalRecord(const uu_id& medicalRecordId) override;

    uu_id createClinic(const std::string& address) override;

    Doctor createDoctor(DoctorSpecialty specialty, const std::vector<uu_id>& clinicIds) override;
    Doctor getDoctor(const uu_id& doctorId) override;

    uu_id createTechnician(const std::string& fullName, const uu_id& clinicId) override;
    uu_id createEquipment(const std::string& model, const uu_id& clinicId) override;
    uu_id createImageType(const std::string& name) override;

    Image uploadImage(
        const std::vector<uint8_t>& imageData,
        const uu_id& patientId,
        const uu_id& techId,
        const uu_id& equipId,
        const uu_id& typeId
    ) override;

    Image getImage(const uu_id& imageId) override;

    Diagnosis createDiagnosis(
        const std::string& text,
        const uu_id& imageId,
        const uu_id& doctorId
    ) override;

    Diagnosis getDiagnosis(const uu_id& diagnosisId) override;

    void showDictionaries() override;

    void deleteClinic(const uu_id& clinicId) override;
    void deleteDoctor(const uu_id& doctorId) override;
    void deleteTechnician(const uu_id& technicianId) override;
    void deleteEquipment(const uu_id& equipmentId) override;
    void deleteImageType(const uu_id& typeId) override;
    void deleteDiagnosis(const uu_id& diagnosisId) override;

    TableData getClinicsTable() override;
    TableData getClinicTable(const uu_id& clinicId) override;

    TableData getDoctorsTable() override;
    TableData getDoctorTable(const uu_id& doctorId) override;

    TableData getTechniciansTable() override;
    TableData getTechnicianTable(const uu_id& technicianId) override;

    TableData getEquipmentTable() override;
    TableData getEquipmentItemTable(const uu_id& equipmentId) override;

    TableData getImageTypesTable() override;
    TableData getImageTypeTable(const uu_id& typeId) override;

    TableData getPatientsWithRecordsTable() override;
    TableData getDiagnosesTable() override;
    TableData getDiagnosisTable(const uu_id& diagnosisId) override;

private:
    std::shared_ptr<pqxx::connection> conn;

    std::shared_ptr<IPatientRepository> patientRepo;
    std::shared_ptr<IDoctorRepository> doctorRepo;
    std::shared_ptr<IMedicalRecordRepository> medicalRecordRepo;
    std::shared_ptr<IImageRepository> imageRepo;
    std::shared_ptr<IDiagnosisRepository> diagnosisRepo;

    std::shared_ptr<ImageService> imageService;
    std::shared_ptr<DiagnosisService> diagnosisService;

    std::shared_ptr<Logger> logger;
    int maxDiagnosisTextLength;

    std::shared_ptr<ICache> cache;
    int cacheTtlSeconds;

    static std::string escapeSqlString(const std::string& value);

    static std::string patientCacheKey(const uu_id& patientId);
    static std::string serializePatient(const Patient& patient);
    static Patient deserializePatient(const std::string& value);
    static std::string dateToString(const std::tm& date);
    static std::tm stringToDate(const std::string& value);

    TableData executeTableQuery(
        const std::vector<std::string>& headers,
        const std::string& sql
    );

    void executeDeleteById(
        const std::string& tableName,
        const uu_id& id,
        const std::string& entityName
    );
};

#endif // APPLICATIONSERVICE_H
