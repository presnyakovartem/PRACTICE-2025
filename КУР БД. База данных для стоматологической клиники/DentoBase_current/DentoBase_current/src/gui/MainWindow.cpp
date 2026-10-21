#include "gui/MainWindow.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QStringList>
#include <QTableWidgetItem>

MainWindow::MainWindow(
    std::shared_ptr<IApplicationService> appService,
    std::shared_ptr<Logger> logger,
    QWidget* parent
)
    : QMainWindow(parent),
      appService(std::move(appService)),
      logger(std::move(logger)) {
    setWindowTitle("DentoBase");
    resize(1200, 780);

    applyModernStyle();

    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    auto* title = new QLabel("DentoBase — система работы со стоматологическими снимками");
    title->setObjectName("titleLabel");

    tabs = new QTabWidget(central);
    tabs->setDocumentMode(true);

    output = new QPlainTextEdit(central);
    output->setReadOnly(true);
    output->setMaximumHeight(150);
    output->setPlaceholderText("Здесь отображается результат операций...");

    tabs->addTab(createPatientsTab(), "Пациенты");
    tabs->addTab(createMedicalRecordsTab(), "Медкарты");
    tabs->addTab(createClinicsTab(), "Клиники");
    tabs->addTab(createDoctorsTab(), "Врачи");
    tabs->addTab(createTechniciansTab(), "Лаборанты");
    tabs->addTab(createEquipmentTab(), "Оборудование");
    tabs->addTab(createImageTypesTab(), "Типы снимков");
    tabs->addTab(createImagesTab(), "Снимки");
    tabs->addTab(createDiagnosesTab(), "Заключения");
    tabs->addTab(createDirectoriesTab(), "Справочники");

    mainLayout->addWidget(title);
    mainLayout->addWidget(tabs);
    mainLayout->addWidget(new QLabel("Журнал действий:"));
    mainLayout->addWidget(output);

    setCentralWidget(central);

    this->logger->info("Qt GUI started");
}

void MainWindow::applyModernStyle() {
    setStyleSheet(
        "QMainWindow { background: #f5f7fb; }"

        "QLabel#titleLabel { "
        "font-size: 22px; "
        "font-weight: 700; "
        "color: #1f2937; "
        "padding: 6px; "
        "}"

        "QTabWidget::pane { "
        "border: 1px solid #d1d5db; "
        "border-radius: 10px; "
        "background: white; "
        "}"

        "QTabBar::tab { "
        "background: #e5e7eb; "
        "padding: 9px 14px; "
        "margin-right: 3px; "
        "border-top-left-radius: 8px; "
        "border-top-right-radius: 8px; "
        "}"

        "QTabBar::tab:selected { "
        "background: white; "
        "color: #2563eb; "
        "font-weight: 600; "
        "}"

        "QGroupBox { "
        "font-size: 15px; "
        "font-weight: 600; "
        "border: 1px solid #d1d5db; "
        "border-radius: 12px; "
        "margin-top: 14px; "
        "padding: 14px; "
        "background: #ffffff; "
        "}"

        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 12px; "
        "padding: 0 6px; "
        "color: #111827; "
        "}"

        "QLineEdit { "
        "border: 1px solid #cbd5e1; "
        "border-radius: 8px; "
        "padding: 8px; "
        "background: #ffffff; "
        "}"

        "QLineEdit:focus { "
        "border: 1px solid #2563eb; "
        "}"

        "QPushButton { "
        "background: #2563eb; "
        "color: white; "
        "border: none; "
        "border-radius: 8px; "
        "padding: 9px 12px; "
        "font-weight: 600; "
        "}"

        "QPushButton:hover { background: #1d4ed8; }"
        "QPushButton:pressed { background: #1e40af; }"

        "QPushButton#dangerButton { background: #dc2626; }"
        "QPushButton#dangerButton:hover { background: #b91c1c; }"

        "QPlainTextEdit { "
        "border: 1px solid #d1d5db; "
        "border-radius: 10px; "
        "background: #0f172a; "
        "color: #e5e7eb; "
        "padding: 8px; "
        "}"

        "QComboBox { "
        "border: 1px solid #cbd5e1; "
        "border-radius: 8px; "
        "padding: 8px; "
        "background: #ffffff; "
        "}"

        "QTableWidget { "
        "background: white; "
        "border: 1px solid #d1d5db; "
        "border-radius: 10px; "
        "gridline-color: #e5e7eb; "
        "}"

        "QHeaderView::section { "
        "background: #f3f4f6; "
        "padding: 8px; "
        "border: none; "
        "font-weight: 600; "
        "}"
    );
}

QGroupBox* MainWindow::createCard(const QString& title, QVBoxLayout** outLayout) {
    auto* group = new QGroupBox(title);
    auto* layout = new QVBoxLayout(group);
    layout->setSpacing(8);
    *outLayout = layout;
    return group;
}

QLineEdit* MainWindow::addInput(QVBoxLayout* layout, const QString& label) {
    layout->addWidget(new QLabel(label));
    auto* edit = new QLineEdit();
    layout->addWidget(edit);
    return edit;
}

void MainWindow::appendOutput(const QString& text) {
    output->appendPlainText("------------------------------");
    output->appendPlainText(text);
}

void MainWindow::showError(const QString& operation, const std::exception& e) {
    logger->error(operation.toStdString() + " failed: " + e.what());
    QMessageBox::critical(this, "Ошибка", operation + ":\n" + e.what());
}

void MainWindow::fillTable(QTableWidget* table, const TableData& data) {
    table->clear();

    table->setColumnCount(static_cast<int>(data.headers.size()));
    table->setRowCount(static_cast<int>(data.rows.size()));

    QStringList headers;
    for (const auto& header : data.headers) {
        headers << QString::fromStdString(header);
    }

    table->setHorizontalHeaderLabels(headers);

    for (int row = 0; row < static_cast<int>(data.rows.size()); ++row) {
        for (int col = 0; col < static_cast<int>(data.rows[row].size()); ++col) {
            table->setItem(
                row,
                col,
                new QTableWidgetItem(QString::fromStdString(data.rows[row][col]))
            );
        }
    }

    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
}

void MainWindow::showTableWindow(const QString& title, const TableData& data) {
    auto* window = new QWidget();
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowTitle(title);
    window->resize(1000, 600);

    auto* layout = new QVBoxLayout(window);
    auto* table = new QTableWidget(window);

    fillTable(table, data);

    layout->addWidget(table);
    window->show();
}

QWidget* MainWindow::createPatientsTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* createLayout = nullptr;
    auto* createCard = this->createCard("Создание пациента", &createLayout);

    auto* nameEdit = addInput(createLayout, "ФИО пациента:");
    auto* birthDateEdit = addInput(createLayout, "Дата рождения YYYY-MM-DD:");

    auto* createButton = new QPushButton("Создать пациента");
    createLayout->addWidget(createButton);

    QVBoxLayout* findLayout = nullptr;
    auto* findCard = this->createCard("Поиск пациента", &findLayout);

    auto* patientIdEdit = addInput(findLayout, "ID пациента:");
    auto* findButton = new QPushButton("Найти пациента");
    auto* listButton = new QPushButton("Показать пациентов и медкарты");

    findLayout->addWidget(findButton);
    findLayout->addWidget(listButton);

    root->addWidget(createCard);
    root->addWidget(findCard);
    root->addStretch();

    QObject::connect(createButton, &QPushButton::clicked, [=]() {
        try {
            Patient patient = appService->createPatient(
                nameEdit->text().toStdString(),
                parseDate(birthDateEdit->text())
            );

            appendOutput(
                "Пациент создан\nID: " + QString::fromStdString(patient.id.value) +
                "\nФИО: " + QString::fromStdString(patient.fullName)
            );
        } catch (const std::exception& e) {
            showError("Создание пациента", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            Patient patient = appService->findPatient(makeUuid(patientIdEdit->text()));

            appendOutput(
                "Пациент найден\nID: " + QString::fromStdString(patient.id.value) +
                "\nФИО: " + QString::fromStdString(patient.fullName)
            );
        } catch (const std::exception& e) {
            showError("Поиск пациента", e);
        }
    });

    QObject::connect(listButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Пациенты и медицинские карты", appService->getPatientsWithRecordsTable());
        } catch (const std::exception& e) {
            showError("Просмотр пациентов", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createMedicalRecordsTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* createLayout = nullptr;
    auto* createCard = this->createCard("Создание медицинской карты", &createLayout);

    auto* patientIdEdit = addInput(createLayout, "ID пациента:");
    auto* notesEdit = addInput(createLayout, "Примечания:");

    auto* createButton = new QPushButton("Создать медкарту");
    createLayout->addWidget(createButton);

    QVBoxLayout* findLayout = nullptr;
    auto* findCard = this->createCard("Поиск медицинской карты", &findLayout);

    auto* recordIdEdit = addInput(findLayout, "ID медкарты:");
    auto* findButton = new QPushButton("Найти медкарту");

    findLayout->addWidget(findButton);

    root->addWidget(createCard);
    root->addWidget(findCard);
    root->addStretch();

    QObject::connect(createButton, &QPushButton::clicked, [=]() {
        try {
            MedicalRecord record = appService->createMedicalRecord(
                makeUuid(patientIdEdit->text()),
                notesEdit->text().toStdString()
            );

            appendOutput(
                "Медкарта создана\nID: " + QString::fromStdString(record.id.value) +
                "\nID пациента: " + QString::fromStdString(record.patientId.value)
            );
        } catch (const std::exception& e) {
            showError("Создание медкарты", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            MedicalRecord record = appService->getMedicalRecord(makeUuid(recordIdEdit->text()));

            appendOutput(
                "Медкарта найдена\nID: " + QString::fromStdString(record.id.value) +
                "\nID пациента: " + QString::fromStdString(record.patientId.value) +
                "\nПримечания: " + QString::fromStdString(record.notes)
            );
        } catch (const std::exception& e) {
            showError("Поиск медкарты", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createClinicsTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* createLayout = nullptr;
    auto* createCard = this->createCard("Создание клиники", &createLayout);

    auto* addressEdit = addInput(createLayout, "Адрес клиники:");
    auto* createButton = new QPushButton("Создать клинику");
    createLayout->addWidget(createButton);

    QVBoxLayout* manageLayout = nullptr;
    auto* manageCard = this->createCard("Поиск и удаление клиники", &manageLayout);

    auto* idEdit = addInput(manageLayout, "ID клиники:");
    auto* findButton = new QPushButton("Найти клинику");
    auto* deleteButton = new QPushButton("Удалить клинику");
    deleteButton->setObjectName("dangerButton");

    manageLayout->addWidget(findButton);
    manageLayout->addWidget(deleteButton);

    auto* listButton = new QPushButton("Показать все клиники");

    root->addWidget(createCard);
    root->addWidget(manageCard);
    root->addWidget(listButton);
    root->addStretch();

    QObject::connect(createButton, &QPushButton::clicked, [=]() {
        try {
            uu_id id = appService->createClinic(addressEdit->text().toStdString());
            appendOutput("Клиника создана\nID: " + QString::fromStdString(id.value));
        } catch (const std::exception& e) {
            showError("Создание клиники", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Клиника", appService->getClinicTable(makeUuid(idEdit->text())));
        } catch (const std::exception& e) {
            showError("Поиск клиники", e);
        }
    });

    QObject::connect(deleteButton, &QPushButton::clicked, [=]() {
        try {
            appService->deleteClinic(makeUuid(idEdit->text()));
            appendOutput("Клиника удалена: " + idEdit->text());
        } catch (const std::exception& e) {
            showError("Удаление клиники", e);
        }
    });

    QObject::connect(listButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Клиники", appService->getClinicsTable());
        } catch (const std::exception& e) {
            showError("Просмотр клиник", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createDoctorsTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* createLayout = nullptr;
    auto* createCard = this->createCard("Создание врача", &createLayout);

    auto* specialtyEdit = addInput(createLayout, "Специальность:");
    auto* clinicIdsEdit = addInput(createLayout, "ID клиник через запятую:");

    auto* createButton = new QPushButton("Создать врача");
    createLayout->addWidget(createButton);
    createLayout->addWidget(new QLabel(
        "Специальности: THERAPIST, ORTHODONTIST, SURGEON, PERIODONTIST, "
        "ENDODONTIST, IMPLANTOLOGIST, PEDIATRIC_DENTIST"
    ));

    QVBoxLayout* manageLayout = nullptr;
    auto* manageCard = this->createCard("Поиск и удаление врача", &manageLayout);

    auto* doctorIdEdit = addInput(manageLayout, "ID врача:");
    auto* findButton = new QPushButton("Найти врача");
    auto* deleteButton = new QPushButton("Удалить врача");
    deleteButton->setObjectName("dangerButton");

    manageLayout->addWidget(findButton);
    manageLayout->addWidget(deleteButton);

    auto* listButton = new QPushButton("Показать всех врачей");

    root->addWidget(createCard);
    root->addWidget(manageCard);
    root->addWidget(listButton);
    root->addStretch();

    QObject::connect(createButton, &QPushButton::clicked, [=]() {
        try {
            Doctor doctor = appService->createDoctor(
                parseSpecialty(specialtyEdit->text()),
                parseUuidList(clinicIdsEdit->text())
            );

            appendOutput("Врач создан\nID: " + QString::fromStdString(doctor.id.value));
        } catch (const std::exception& e) {
            showError("Создание врача", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Врач", appService->getDoctorTable(makeUuid(doctorIdEdit->text())));
        } catch (const std::exception& e) {
            showError("Поиск врача", e);
        }
    });

    QObject::connect(deleteButton, &QPushButton::clicked, [=]() {
        try {
            appService->deleteDoctor(makeUuid(doctorIdEdit->text()));
            appendOutput("Врач удалён: " + doctorIdEdit->text());
        } catch (const std::exception& e) {
            showError("Удаление врача", e);
        }
    });

    QObject::connect(listButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Врачи", appService->getDoctorsTable());
        } catch (const std::exception& e) {
            showError("Просмотр врачей", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createTechniciansTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* createLayout = nullptr;
    auto* createCard = this->createCard("Создание лаборанта", &createLayout);

    auto* fullNameEdit = addInput(createLayout, "ФИО лаборанта:");
    auto* clinicIdEdit = addInput(createLayout, "ID клиники:");

    auto* createButton = new QPushButton("Создать лаборанта");
    createLayout->addWidget(createButton);

    QVBoxLayout* manageLayout = nullptr;
    auto* manageCard = this->createCard("Поиск и удаление лаборанта", &manageLayout);

    auto* technicianIdEdit = addInput(manageLayout, "ID лаборанта:");
    auto* findButton = new QPushButton("Найти лаборанта");
    auto* deleteButton = new QPushButton("Удалить лаборанта");
    deleteButton->setObjectName("dangerButton");

    manageLayout->addWidget(findButton);
    manageLayout->addWidget(deleteButton);

    auto* listButton = new QPushButton("Показать всех лаборантов");

    root->addWidget(createCard);
    root->addWidget(manageCard);
    root->addWidget(listButton);
    root->addStretch();

    QObject::connect(createButton, &QPushButton::clicked, [=]() {
        try {
            uu_id id = appService->createTechnician(
                fullNameEdit->text().toStdString(),
                makeUuid(clinicIdEdit->text())
            );

            appendOutput("Лаборант создан\nID: " + QString::fromStdString(id.value));
        } catch (const std::exception& e) {
            showError("Создание лаборанта", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Лаборант", appService->getTechnicianTable(makeUuid(technicianIdEdit->text())));
        } catch (const std::exception& e) {
            showError("Поиск лаборанта", e);
        }
    });

    QObject::connect(deleteButton, &QPushButton::clicked, [=]() {
        try {
            appService->deleteTechnician(makeUuid(technicianIdEdit->text()));
            appendOutput("Лаборант удалён: " + technicianIdEdit->text());
        } catch (const std::exception& e) {
            showError("Удаление лаборанта", e);
        }
    });

    QObject::connect(listButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Лаборанты", appService->getTechniciansTable());
        } catch (const std::exception& e) {
            showError("Просмотр лаборантов", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createEquipmentTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* createLayout = nullptr;
    auto* createCard = this->createCard("Создание оборудования", &createLayout);

    auto* modelEdit = addInput(createLayout, "Модель оборудования:");
    auto* clinicIdEdit = addInput(createLayout, "ID клиники:");

    auto* createButton = new QPushButton("Создать оборудование");
    createLayout->addWidget(createButton);

    QVBoxLayout* manageLayout = nullptr;
    auto* manageCard = this->createCard("Поиск и удаление оборудования", &manageLayout);

    auto* equipmentIdEdit = addInput(manageLayout, "ID оборудования:");
    auto* findButton = new QPushButton("Найти оборудование");
    auto* deleteButton = new QPushButton("Удалить оборудование");
    deleteButton->setObjectName("dangerButton");

    manageLayout->addWidget(findButton);
    manageLayout->addWidget(deleteButton);

    auto* listButton = new QPushButton("Показать всё оборудование");

    root->addWidget(createCard);
    root->addWidget(manageCard);
    root->addWidget(listButton);
    root->addStretch();

    QObject::connect(createButton, &QPushButton::clicked, [=]() {
        try {
            uu_id id = appService->createEquipment(
                modelEdit->text().toStdString(),
                makeUuid(clinicIdEdit->text())
            );

            appendOutput("Оборудование создано\nID: " + QString::fromStdString(id.value));
        } catch (const std::exception& e) {
            showError("Создание оборудования", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Оборудование", appService->getEquipmentItemTable(makeUuid(equipmentIdEdit->text())));
        } catch (const std::exception& e) {
            showError("Поиск оборудования", e);
        }
    });

    QObject::connect(deleteButton, &QPushButton::clicked, [=]() {
        try {
            appService->deleteEquipment(makeUuid(equipmentIdEdit->text()));
            appendOutput("Оборудование удалено: " + equipmentIdEdit->text());
        } catch (const std::exception& e) {
            showError("Удаление оборудования", e);
        }
    });

    QObject::connect(listButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Оборудование", appService->getEquipmentTable());
        } catch (const std::exception& e) {
            showError("Просмотр оборудования", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createImageTypesTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* createLayout = nullptr;
    auto* createCard = this->createCard("Создание типа снимка", &createLayout);

    auto* nameEdit = addInput(createLayout, "Название типа снимка:");

    auto* createButton = new QPushButton("Создать тип снимка");
    createLayout->addWidget(createButton);

    QVBoxLayout* manageLayout = nullptr;
    auto* manageCard = this->createCard("Поиск и удаление типа снимка", &manageLayout);

    auto* typeIdEdit = addInput(manageLayout, "ID типа снимка:");
    auto* findButton = new QPushButton("Найти тип снимка");
    auto* deleteButton = new QPushButton("Удалить тип снимка");
    deleteButton->setObjectName("dangerButton");

    manageLayout->addWidget(findButton);
    manageLayout->addWidget(deleteButton);

    auto* listButton = new QPushButton("Показать все типы снимков");

    root->addWidget(createCard);
    root->addWidget(manageCard);
    root->addWidget(listButton);
    root->addStretch();

    QObject::connect(createButton, &QPushButton::clicked, [=]() {
        try {
            uu_id id = appService->createImageType(nameEdit->text().toStdString());
            appendOutput("Тип снимка создан\nID: " + QString::fromStdString(id.value));
        } catch (const std::exception& e) {
            showError("Создание типа снимка", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Тип снимка", appService->getImageTypeTable(makeUuid(typeIdEdit->text())));
        } catch (const std::exception& e) {
            showError("Поиск типа снимка", e);
        }
    });

    QObject::connect(deleteButton, &QPushButton::clicked, [=]() {
        try {
            appService->deleteImageType(makeUuid(typeIdEdit->text()));
            appendOutput("Тип снимка удалён: " + typeIdEdit->text());
        } catch (const std::exception& e) {
            showError("Удаление типа снимка", e);
        }
    });

    QObject::connect(listButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Типы снимков", appService->getImageTypesTable());
        } catch (const std::exception& e) {
            showError("Просмотр типов снимков", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createImagesTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* uploadLayout = nullptr;
    auto* uploadCard = this->createCard("Загрузка снимка", &uploadLayout);

    auto* filePathEdit = addInput(uploadLayout, "Путь к файлу снимка:");
    auto* patientIdEdit = addInput(uploadLayout, "ID пациента:");
    auto* technicianIdEdit = addInput(uploadLayout, "ID лаборанта:");
    auto* equipmentIdEdit = addInput(uploadLayout, "ID оборудования:");
    auto* typeIdEdit = addInput(uploadLayout, "ID типа снимка:");

    auto* chooseFileButton = new QPushButton("Выбрать файл");
    auto* uploadButton = new QPushButton("Загрузить снимок");

    uploadLayout->addWidget(chooseFileButton);
    uploadLayout->addWidget(uploadButton);

    QVBoxLayout* findLayout = nullptr;
    auto* findCard = this->createCard("Поиск снимка", &findLayout);

    auto* imageIdEdit = addInput(findLayout, "ID снимка:");
    auto* findButton = new QPushButton("Найти снимок");

    findLayout->addWidget(findButton);

    root->addWidget(uploadCard);
    root->addWidget(findCard);
    root->addStretch();

    QObject::connect(chooseFileButton, &QPushButton::clicked, [=]() {
        QString path = QFileDialog::getOpenFileName(this, "Выбрать снимок");
        if (!path.isEmpty()) {
            filePathEdit->setText(path);
        }
    });

    QObject::connect(uploadButton, &QPushButton::clicked, [=]() {
        try {
            Image image = appService->uploadImage(
                readFileBytes(filePathEdit->text()),
                makeUuid(patientIdEdit->text()),
                makeUuid(technicianIdEdit->text()),
                makeUuid(equipmentIdEdit->text()),
                makeUuid(typeIdEdit->text())
            );

            appendOutput("Снимок загружен\nID: " + QString::fromStdString(image.id.value));
        } catch (const std::exception& e) {
            showError("Загрузка снимка", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            Image image = appService->getImage(makeUuid(imageIdEdit->text()));

            appendOutput(
                "Снимок найден\nID: " + QString::fromStdString(image.id.value) +
                "\nРазмер: " + QString::number(image.imageData.size()) + " байт" +
                "\nID пациента: " + QString::fromStdString(image.patientId.value)
            );
        } catch (const std::exception& e) {
            showError("Поиск снимка", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createDiagnosesTab() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);

    QVBoxLayout* createLayout = nullptr;
    auto* createCard = this->createCard("Создание заключения", &createLayout);

    auto* textEdit = addInput(createLayout, "Текст заключения:");
    auto* imageIdEdit = addInput(createLayout, "ID снимка:");
    auto* doctorIdEdit = addInput(createLayout, "ID врача:");

    auto* createButton = new QPushButton("Создать заключение");
    createLayout->addWidget(createButton);

    QVBoxLayout* manageLayout = nullptr;
    auto* manageCard = this->createCard("Поиск и удаление заключения", &manageLayout);

    auto* diagnosisIdEdit = addInput(manageLayout, "ID заключения:");
    auto* findButton = new QPushButton("Найти заключение");
    auto* deleteButton = new QPushButton("Удалить заключение");
    deleteButton->setObjectName("dangerButton");

    manageLayout->addWidget(findButton);
    manageLayout->addWidget(deleteButton);

    auto* listButton = new QPushButton("Показать все заключения");

    root->addWidget(createCard);
    root->addWidget(manageCard);
    root->addWidget(listButton);
    root->addStretch();

    QObject::connect(createButton, &QPushButton::clicked, [=]() {
        try {
            Diagnosis diagnosis = appService->createDiagnosis(
                textEdit->text().toStdString(),
                makeUuid(imageIdEdit->text()),
                makeUuid(doctorIdEdit->text())
            );

            appendOutput("Заключение создано\nID: " + QString::fromStdString(diagnosis.id.value));
        } catch (const std::exception& e) {
            showError("Создание заключения", e);
        }
    });

    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Заключение", appService->getDiagnosisTable(makeUuid(diagnosisIdEdit->text())));
        } catch (const std::exception& e) {
            showError("Поиск заключения", e);
        }
    });

    QObject::connect(deleteButton, &QPushButton::clicked, [=]() {
        try {
            appService->deleteDiagnosis(makeUuid(diagnosisIdEdit->text()));
            appendOutput("Заключение удалено: " + diagnosisIdEdit->text());
        } catch (const std::exception& e) {
            showError("Удаление заключения", e);
        }
    });

    QObject::connect(listButton, &QPushButton::clicked, [=]() {
        try {
            showTableWindow("Заключения", appService->getDiagnosesTable());
        } catch (const std::exception& e) {
            showError("Просмотр заключений", e);
        }
    });

    return widget;
}

QWidget* MainWindow::createDirectoriesTab() {
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);

    QVBoxLayout* cardLayout = nullptr;
    auto* card = this->createCard("Просмотр отдельных справочников", &cardLayout);

    auto* combo = new QComboBox();
    combo->addItem("Клиники");
    combo->addItem("Врачи");
    combo->addItem("Лаборанты");
    combo->addItem("Оборудование");
    combo->addItem("Типы снимков");
    combo->addItem("Пациенты и медкарты");
    combo->addItem("Заключения");

    auto* button = new QPushButton("Открыть выбранный справочник");

    cardLayout->addWidget(new QLabel("Выберите справочник:"));
    cardLayout->addWidget(combo);
    cardLayout->addWidget(button);

    layout->addWidget(card);
    layout->addStretch();

    QObject::connect(button, &QPushButton::clicked, [=]() {
        try {
            QString name = combo->currentText();

            if (name == "Клиники") {
                showTableWindow("Клиники", appService->getClinicsTable());
            } else if (name == "Врачи") {
                showTableWindow("Врачи", appService->getDoctorsTable());
            } else if (name == "Лаборанты") {
                showTableWindow("Лаборанты", appService->getTechniciansTable());
            } else if (name == "Оборудование") {
                showTableWindow("Оборудование", appService->getEquipmentTable());
            } else if (name == "Типы снимков") {
                showTableWindow("Типы снимков", appService->getImageTypesTable());
            } else if (name == "Пациенты и медкарты") {
                showTableWindow("Пациенты и медкарты", appService->getPatientsWithRecordsTable());
            } else if (name == "Заключения") {
                showTableWindow("Заключения", appService->getDiagnosesTable());
            }
        } catch (const std::exception& e) {
            showError("Открытие справочника", e);
        }
    });

    return widget;
}

uu_id MainWindow::makeUuid(const QString& value) {
    if (value.trimmed().isEmpty()) {
        throw std::invalid_argument("UUID is empty");
    }

    uu_id id;
    id.value = value.trimmed().toStdString();
    return id;
}

std::tm MainWindow::parseDate(const QString& value) {
    std::tm date{};
    std::istringstream ss(value.toStdString());
    ss >> std::get_time(&date, "%Y-%m-%d");

    if (ss.fail()) {
        throw std::invalid_argument("Invalid date format. Expected YYYY-MM-DD");
    }

    return date;
}

DoctorSpecialty MainWindow::parseSpecialty(const QString& value) {
    std::string v = value.trimmed().toStdString();

    if (v == "THERAPIST") {
        return DoctorSpecialty::THERAPIST;
    }

    if (v == "ORTHODONTIST") {
        return DoctorSpecialty::ORTHODONTIST;
    }

    if (v == "SURGEON") {
        return DoctorSpecialty::SURGEON;
    }

    if (v == "PERIODONTIST") {
        return DoctorSpecialty::PERIODONTIST;
    }

    if (v == "ENDODONTIST") {
        return DoctorSpecialty::ENDODONTIST;
    }

    if (v == "IMPLANTOLOGIST") {
        return DoctorSpecialty::IMPLANTOLOGIST;
    }

    if (v == "PEDIATRIC_DENTIST") {
        return DoctorSpecialty::PEDIATRIC_DENTIST;
    }

    throw std::invalid_argument("Unknown doctor specialty");
}

std::vector<uu_id> MainWindow::parseUuidList(const QString& value) {
    std::vector<uu_id> result;

    QStringList parts = value.split(",", Qt::SkipEmptyParts);

    for (const auto& part : parts) {
        result.push_back(makeUuid(part));
    }

    if (result.empty()) {
        throw std::invalid_argument("Clinic list is empty");
    }

    return result;
}

std::vector<uint8_t> MainWindow::readFileBytes(const QString& filePath) {
    std::ifstream file(filePath.toStdString(), std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file");
    }

    std::vector<uint8_t> data(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    if (data.empty()) {
        throw std::runtime_error("File is empty");
    }

    return data;
}