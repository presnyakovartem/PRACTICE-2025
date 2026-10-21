#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <vector>

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "app/IApplicationService.h"
#include "logging/Logger.h"

class MainWindow : public QMainWindow {
public:
    MainWindow(
        std::shared_ptr<IApplicationService> appService,
        std::shared_ptr<Logger> logger,
        QWidget* parent = nullptr
    );

private:
    std::shared_ptr<IApplicationService> appService;
    std::shared_ptr<Logger> logger;

    QTabWidget* tabs;
    QPlainTextEdit* output;

    QWidget* createPatientsTab();
    QWidget* createMedicalRecordsTab();
    QWidget* createClinicsTab();
    QWidget* createDoctorsTab();
    QWidget* createTechniciansTab();
    QWidget* createEquipmentTab();
    QWidget* createImageTypesTab();
    QWidget* createImagesTab();
    QWidget* createDiagnosesTab();
    QWidget* createDirectoriesTab();

    QLineEdit* addInput(QVBoxLayout* layout, const QString& label);
    QGroupBox* createCard(const QString& title, QVBoxLayout** outLayout);

    void appendOutput(const QString& text);
    void showError(const QString& operation, const std::exception& e);

    void showTableWindow(const QString& title, const TableData& data);
    void fillTable(QTableWidget* table, const TableData& data);

    static uu_id makeUuid(const QString& value);
    static std::tm parseDate(const QString& value);
    static DoctorSpecialty parseSpecialty(const QString& value);
    static std::vector<uu_id> parseUuidList(const QString& value);
    static std::vector<uint8_t> readFileBytes(const QString& filePath);

    void applyModernStyle();
};

#endif // MAINWINDOW_H