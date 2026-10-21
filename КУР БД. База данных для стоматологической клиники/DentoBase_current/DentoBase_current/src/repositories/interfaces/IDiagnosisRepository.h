#ifndef IDIAGNOSISREPOSITORY_H
#define IDIAGNOSISREPOSITORY_H

#include "domain/Diagnosis.h"

class IDiagnosisRepository {
public:
    virtual ~IDiagnosisRepository() = default;
    virtual void add(const Diagnosis& diagnosis) = 0;
    virtual Diagnosis getById(const uu_id& id) = 0;
};

#endif // IDIAGNOSISREPOSITORY_H