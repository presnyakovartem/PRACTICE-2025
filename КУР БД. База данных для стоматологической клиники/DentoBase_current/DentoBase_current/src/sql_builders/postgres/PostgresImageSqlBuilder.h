#ifndef POSTGRESIMAGESQLBUILDER_H
#define POSTGRESIMAGESQLBUILDER_H

#include "sql_builders/interfaces/IImageSqlBuilder.h"

class PostgresImageSqlBuilder : public IImageSqlBuilder {
public:
    std::string buildSelectById(const uu_id& id) const override;
    std::string buildInsert(const Image& image) const override;
};

#endif