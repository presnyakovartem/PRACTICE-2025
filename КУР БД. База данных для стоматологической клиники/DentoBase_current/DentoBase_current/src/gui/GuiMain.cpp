#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <QApplication>
#include <QMessageBox>
#include <pqxx/pqxx>

#include "cache/ICache.h"
#include "cache/RedisCache.h"
#include "config/Config.h"
#include "logging/Logger.h"

#include "repositories/postgres/PostgresPatientRepository.h"
#include "repositories/postgres/PostgresDoctorRepository.h"
#include "repositories/postgres/PostgresMedicalRecordRepository.h"
#include "repositories/postgres/PostgresImageRepository.h"
#include "repositories/postgres/PostgresDiagnosisRepository.h"

#include "sql_builders/postgres/PostgresPatientSqlBuilder.h"
#include "sql_builders/postgres/PostgresDoctorSqlBuilder.h"
#include "sql_builders/postgres/PostgresMedicalRecordSqlBuilder.h"
#include "sql_builders/postgres/PostgresImageSqlBuilder.h"
#include "sql_builders/postgres/PostgresDiagnosisSqlBuilder.h"

#include "services/ImageService.h"
#include "services/DiagnosisService.h"

#include "app/ApplicationService.h"
#include "gui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication qtApp(argc, argv);

    if (argc < 2) {
        QMessageBox::critical(nullptr, "Ошибка запуска", "Usage: dentobase_gui_app <config_path>");
        std::cerr << "Usage: dentobase_gui_app <config_path>" << std::endl;
        return 1;
    }

    try {
        const std::string configPath = argv[1];
        Config config(configPath);

        const std::string logFilePath = config.getString("log.file", "logs/gui.log");
        const std::string logLevel = config.getString("log.level", "release");

        auto logger = std::make_shared<Logger>(
            logFilePath,
            Logger::fromString(logLevel)
        );

        logger->info("DentoBase GUI application started");
        logger->info("Config file: " + configPath);
        logger->info("Log file: " + logFilePath);
        logger->info("Logging mode from config: " + logLevel);

        const std::string connectionString = config.getString("db.connection_string");
        if (connectionString.empty()) {
            throw std::runtime_error("Database connection string is empty");
        }

        auto conn = std::make_shared<pqxx::connection>(connectionString);
        if (!conn->is_open()) {
            throw std::runtime_error("PostgreSQL connection is not open");
        }
        logger->info("Connected to PostgreSQL");

        std::shared_ptr<ICache> cache = nullptr;
        int redisTtlSeconds = config.getInt("redis.ttl_seconds", 300);

        const std::string redisEnabled = config.getString("redis.enabled", "true");
        if (redisEnabled != "false" && redisEnabled != "0") {
            try {
                cache = std::make_shared<RedisCache>(
                    config.getString("redis.host", "127.0.0.1"),
                    config.getInt("redis.port", 6379),
                    logger
                );
                logger->info("Connected to Redis");
            } catch (const std::exception& e) {
                logger->warning(std::string("Redis unavailable, application will continue without cache: ") + e.what());
            }
        } else {
            logger->info("Redis cache disabled by config");
        }

        auto patientSqlBuilder = std::make_shared<PostgresPatientSqlBuilder>();
        auto doctorSqlBuilder = std::make_shared<PostgresDoctorSqlBuilder>();
        auto medicalRecordSqlBuilder = std::make_shared<PostgresMedicalRecordSqlBuilder>();
        auto imageSqlBuilder = std::make_shared<PostgresImageSqlBuilder>();
        auto diagnosisSqlBuilder = std::make_shared<PostgresDiagnosisSqlBuilder>();

        auto patientRepo = std::make_shared<PostgresPatientRepository>(conn, patientSqlBuilder);
        auto doctorRepo = std::make_shared<PostgresDoctorRepository>(conn, doctorSqlBuilder);
        auto medicalRecordRepo = std::make_shared<PostgresMedicalRecordRepository>(conn, medicalRecordSqlBuilder);
        auto imageRepo = std::make_shared<PostgresImageRepository>(conn, imageSqlBuilder);
        auto diagnosisRepo = std::make_shared<PostgresDiagnosisRepository>(conn, diagnosisSqlBuilder);

        auto imageService = std::make_shared<ImageService>(imageRepo, patientRepo);
        auto diagnosisService = std::make_shared<DiagnosisService>(
            diagnosisRepo,
            doctorRepo,
            imageRepo,
            medicalRecordRepo
        );

        auto appService = std::make_shared<ApplicationService>(
            conn,
            patientRepo,
            doctorRepo,
            medicalRecordRepo,
            imageRepo,
            diagnosisRepo,
            imageService,
            diagnosisService,
            logger,
            config.getInt("business.max_diagnosis_text_length", 2048),
            cache,
            redisTtlSeconds
        );

        MainWindow window(appService, logger);
        window.show();

        int result = qtApp.exec();
        logger->info("DentoBase GUI application finished");
        return result;
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Fatal GUI error", QString::fromStdString(e.what()));
        std::cerr << "Fatal GUI error: " << e.what() << std::endl;
        return 1;
    }
}
