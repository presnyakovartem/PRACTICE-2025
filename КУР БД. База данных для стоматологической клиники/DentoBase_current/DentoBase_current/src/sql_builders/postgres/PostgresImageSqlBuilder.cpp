#include "PostgresImageSqlBuilder.h"
#include "repositories/postgres/DbUtils.h"  // vectorToByteaHex
#include <sstream>

std::string PostgresImageSqlBuilder::buildSelectById(const uu_id& id) const {
    std::stringstream sql;
    sql << "SELECT id, image_data, extract(epoch from upload_date), patient_id, tech_id, equip_id, type_id "
        << "FROM images WHERE id = '" << id.value << "'";
    return sql.str();
}

std::string PostgresImageSqlBuilder::buildInsert(const Image& image) const {
    std::stringstream sql;
    sql << "INSERT INTO images (id, image_data, upload_date, patient_id, tech_id, equip_id, type_id) VALUES ('"
        << image.id.value << "', "
        << vectorToByteaHex(image.imageData) << ", "
        << "to_timestamp(" << image.uploadDate << "), '"
        << image.patientId.value << "', '"
        << image.techId.value << "', '"
        << image.equipId.value << "', '"
        << image.typeId.value << "')";
    return sql.str();
}