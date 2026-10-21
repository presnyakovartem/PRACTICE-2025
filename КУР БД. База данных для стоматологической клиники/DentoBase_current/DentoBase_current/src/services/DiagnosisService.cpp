#include "DiagnosisService.h"
#include <ctime>
#include <stdexcept>

DiagnosisService::DiagnosisService(std::shared_ptr<IDiagnosisRepository> diagRepo,
                                   std::shared_ptr<IDoctorRepository> docRepo,
                                   std::shared_ptr<IImageRepository> imgRepo,
                                   std::shared_ptr<IMedicalRecordRepository> medRecordRepo)
    : diagnosisRepo(diagRepo), doctorRepo(docRepo), imageRepo(imgRepo), medicalRecordRepo(medRecordRepo) {}

Diagnosis DiagnosisService::createDiagnosis(const std::string& text,
                                            const uu_id& imageId,
                                            const uu_id& doctorId) {
    if (text.empty()) {
        throw std::invalid_argument("Diagnosis text cannot be empty");
    }

    Doctor doctor = doctorRepo->getById(doctorId);
    Image image = imageRepo->getById(imageId);
    MedicalRecord record = medicalRecordRepo->getActiveByPatientId(image.patientId);

    Diagnosis d;
    d.id = uu_id::generate();
    d.text = text;
    d.imageId = imageId;
    d.doctorId = doctorId;
    d.medicalRecordId = record.id;
    d.createdAt = std::time(nullptr);

    diagnosisRepo->add(d);
    return d;
}