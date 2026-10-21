#include "PostgresImageRepository.h"
#include "DbUtils.h"
#include <stdexcept>

PostgresImageRepository::PostgresImageRepository(std::shared_ptr<pqxx::connection> conn,
                                                 std::shared_ptr<IImageSqlBuilder> sqlBuilder)
    : m_conn(conn), m_sqlBuilder(sqlBuilder) {}

void PostgresImageRepository::add(const Image& image) {
    if (image.id.isNull()) throw std::invalid_argument("Image ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildInsert(image);
    txn.exec(sql);
    txn.commit();
}

Image PostgresImageRepository::getById(const uu_id& id) {
    if (id.isNull()) throw std::invalid_argument("Image ID is null");
    pqxx::work txn(*m_conn);
    std::string sql = m_sqlBuilder->buildSelectById(id);
    auto res = txn.exec(sql);
    if (res.empty()) throw std::runtime_error("Image not found");
    Image img;
    img.id.value = res[0][0].as<std::string>();
    img.imageData = byteaToVector(res[0][1]);
    img.uploadDate = static_cast<std::time_t>(res[0][2].as<double>());
    img.patientId.value = res[0][3].as<std::string>();
    img.techId.value = res[0][4].as<std::string>();
    img.equipId.value = res[0][5].as<std::string>();
    img.typeId.value = res[0][6].as<std::string>();
    txn.commit();
    return img;
}