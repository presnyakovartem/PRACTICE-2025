#ifndef IMEDICALRECORDREPOSITORY_H
#define IMEDICALRECORDREPOSITORY_H

#include "domain/MedicalRecord.h"

class IMedicalRecordRepository {
public:
    virtual ~IMedicalRecordRepository() = default;
    virtual MedicalRecord getById(const uu_id& id) = 0;
    virtual MedicalRecord getActiveByPatientId(const uu_id& patientId) = 0;
    virtual void add(const MedicalRecord& record) = 0;
    virtual void update(const MedicalRecord& record) = 0;
};

#endif // IMEDICALRECORDREPOSITORY_H