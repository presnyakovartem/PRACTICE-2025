#ifndef POSTGRESDOCTORREPOSITORY_H
#define POSTGRESDOCTORREPOSITORY_H

#include "repositories/interfaces/IDoctorRepository.h"
#include "sql_builders/interfaces/IDoctorSqlBuilder.h"
#include <pqxx/pqxx>
#include <memory>
#include <vector>

class PostgresDoctorRepository : public IDoctorRepository {
public:
    PostgresDoctorRepository(std::shared_ptr<pqxx::connection> conn,
                             std::shared_ptr<IDoctorSqlBuilder> sqlBuilder);
    Doctor getById(const uu_id& id) override;
    void save(const Doctor& doctor) override;

    void addClinic(const uu_id& doctorId, const uu_id& clinicId) override;
    std::vector<uu_id> getClinicIds(const uu_id& doctorId) override;

private:
    std::shared_ptr<pqxx::connection> m_conn;
    std::shared_ptr<IDoctorSqlBuilder> m_sqlBuilder;
};

#endif // POSTGRESDOCTORREPOSITORY_H
