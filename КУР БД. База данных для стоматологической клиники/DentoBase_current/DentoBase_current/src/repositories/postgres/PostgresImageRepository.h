#ifndef POSTGRESIMAGEREPOSITORY_H
#define POSTGRESIMAGEREPOSITORY_H

#include "repositories/interfaces/IImageRepository.h"
#include "sql_builders/interfaces/IImageSqlBuilder.h"
#include <pqxx/pqxx>
#include <memory>

class PostgresImageRepository : public IImageRepository {
public:
    PostgresImageRepository(std::shared_ptr<pqxx::connection> conn,
                            std::shared_ptr<IImageSqlBuilder> sqlBuilder);
    void add(const Image& image) override;
    Image getById(const uu_id& id) override;

private:
    std::shared_ptr<pqxx::connection> m_conn;
    std::shared_ptr<IImageSqlBuilder> m_sqlBuilder;
};

#endif