#ifndef IAPPLICATIONSERVICE_H
#define IAPPLICATIONSERVICE_H

#include <ctime>
#include <cstdint>
#include <string>
#include <vector>

#include "domain/Uuid.h"
#include "domain/Patient.h"
#include "domain/Doctor.h"
#include "domain/MedicalRecord.h"
#include "domain/Image.h"
#include "domain/Diagnosis.h"

struct TableData {
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
};

class IApplicationService {
public:
    virtual ~IApplicationService() = default;

    virtual Patient createPatient(
        const std::string& fullName,
        const std::tm& birthDate
    ) = 0;

    virtual Patient findPatient(
        const uu_id& patientId
    ) = 0;

    virtual MedicalRecord createMedicalRecord(
        const uu_id& patientId,
        const std::string& notes
    ) = 0;

    virtual MedicalRecord getMedicalRecord(
        const uu_id& medicalRecordId
    ) = 0;

    virtual uu_id createClinic(
        const std::string& address
    ) = 0;

    virtual Doctor createDoctor(
        DoctorSpecialty specialty,
        const std::vector<uu_id>& clinicIds
    ) = 0;

    virtual Doctor getDoctor(
        const uu_id& doctorId
    ) = 0;

    virtual uu_id createTechnician(
        const std::string& fullName,
        const uu_id& clinicId
    ) = 0;

    virtual uu_id createEquipment(
        const std::string& model,
        const uu_id& clinicId
    ) = 0;

    virtual uu_id createImageType(
        const std::string& name
    ) = 0;

    virtual Image uploadImage(
        const std::vector<uint8_t>& imageData,
        const uu_id& patientId,
        const uu_id& techId,
        const uu_id& equipId,
        const uu_id& typeId
    ) = 0;

    virtual Image getImage(
        const uu_id& imageId
    ) = 0;

    virtual Diagnosis createDiagnosis(
        const std::string& text,
        const uu_id& imageId,
        const uu_id& doctorId
    ) = 0;

    virtual Diagnosis getDiagnosis(
        const uu_id& diagnosisId
    ) = 0;

    virtual void deleteClinic(const uu_id& clinicId) = 0;
    virtual void deleteDoctor(const uu_id& doctorId) = 0;
    virtual void deleteTechnician(const uu_id& technicianId) = 0;
    virtual void deleteEquipment(const uu_id& equipmentId) = 0;
    virtual void deleteImageType(const uu_id& typeId) = 0;
    virtual void deleteDiagnosis(const uu_id& diagnosisId) = 0;

    virtual TableData getClinicsTable() = 0;
    virtual TableData getClinicTable(const uu_id& clinicId) = 0;

    virtual TableData getDoctorsTable() = 0;
    virtual TableData getDoctorTable(const uu_id& doctorId) = 0;

    virtual TableData getTechniciansTable() = 0;
    virtual TableData getTechnicianTable(const uu_id& technicianId) = 0;

    virtual TableData getEquipmentTable() = 0;
    virtual TableData getEquipmentItemTable(const uu_id& equipmentId) = 0;

    virtual TableData getImageTypesTable() = 0;
    virtual TableData getImageTypeTable(const uu_id& typeId) = 0;

    virtual TableData getPatientsWithRecordsTable() = 0;
    virtual TableData getDiagnosesTable() = 0;
    virtual TableData getDiagnosisTable(const uu_id& diagnosisId) = 0;

    virtual void showDictionaries() = 0;
};

#endif // IAPPLICATIONSERVICE_H