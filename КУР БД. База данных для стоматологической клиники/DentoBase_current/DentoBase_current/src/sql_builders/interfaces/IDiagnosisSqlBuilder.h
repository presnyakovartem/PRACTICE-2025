#ifndef IDIAGNOSISSQLBUILDER_H
#define IDIAGNOSISSQLBUILDER_H

#include <string>
#include "domain/Diagnosis.h"

class IDiagnosisSqlBuilder {
public:
    virtual ~IDiagnosisSqlBuilder() = default;
    virtual std::string buildSelectById(const uu_id& id) const = 0;
    virtual std::string buildInsert(const Diagnosis& diagnosis) const = 0;
};

#endif