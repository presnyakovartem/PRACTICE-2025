#ifndef DIAGNOSISSERVICE_H
#define DIAGNOSISSERVICE_H

#include <memory>
#include <string>
#include "repositories/interfaces/IDiagnosisRepository.h"
#include "repositories/interfaces/IDoctorRepository.h"
#include "repositories/interfaces/IImageRepository.h"
#include "repositories/interfaces/IMedicalRecordRepository.h"
#include "domain/Diagnosis.h"

class DiagnosisService {
public:
    DiagnosisService(std::shared_ptr<IDiagnosisRepository> diagRepo,
                     std::shared_ptr<IDoctorRepository> docRepo,
                     std::shared_ptr<IImageRepository> imgRepo,
                     std::shared_ptr<IMedicalRecordRepository> medRecordRepo);

    Diagnosis createDiagnosis(const std::string& text,
                              const uu_id& imageId,
                              const uu_id& doctorId);

private:
    std::shared_ptr<IDiagnosisRepository> diagnosisRepo;
    std::shared_ptr<IDoctorRepository> doctorRepo;
    std::shared_ptr<IImageRepository> imageRepo;
    std::shared_ptr<IMedicalRecordRepository> medicalRecordRepo;
};

#endif // DIAGNOSISSERVICE_H