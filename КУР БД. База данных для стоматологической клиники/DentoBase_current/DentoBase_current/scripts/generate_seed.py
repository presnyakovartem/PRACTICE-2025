#!/usr/bin/env python3

import os
import random
import uuid
from datetime import date, datetime, timedelta

OUTPUT_PATH = "database/seed.sql"
N = 1000
random.seed(42)


def q(value: str) -> str:
    return "'" + value.replace("'", "''") + "'"


def uid() -> str:
    return str(uuid.uuid4())


def random_date(start_year: int = 1950, end_year: int = 2015) -> str:
    start = date(start_year, 1, 1)
    end = date(end_year, 12, 31)
    return str(start + timedelta(days=random.randint(0, (end - start).days)))


def random_timestamp(start_year: int = 2020, end_year: int = 2026) -> str:
    start = datetime(start_year, 1, 1, 0, 0, 0)
    end = datetime(end_year, 12, 31, 23, 59, 59)
    return str(start + timedelta(seconds=random.randint(0, int((end - start).total_seconds()))))


def bytea_hex(i: int) -> str:
    payload = f"DentoBase test image payload #{i}".encode("utf-8")
    return "\\x" + payload.hex()


def write_insert(file, table, columns, rows, chunk_size=200):
    for start in range(0, len(rows), chunk_size):
        chunk = rows[start:start + chunk_size]
        file.write(f"INSERT INTO {table} ({', '.join(columns)}) VALUES\n")
        values = []
        for row in chunk:
            values.append("    (" + ", ".join(row) + ")")
        file.write(",\n".join(values))
        file.write(";\n\n")


def main():
    os.makedirs("database", exist_ok=True)

    specialties = [
        "THERAPIST", "ORTHODONTIST", "SURGEON", "PERIODONTIST",
        "ENDODONTIST", "IMPLANTOLOGIST", "PEDIATRIC_DENTIST",
    ]
    first_names = ["Иван", "Петр", "Алексей", "Дмитрий", "Сергей", "Анна", "Мария", "Елена", "Ольга", "Наталья"]
    last_names = ["Иванов", "Петров", "Сидоров", "Смирнов", "Кузнецов", "Попов", "Васильев", "Соколов", "Михайлов", "Федоров"]
    patronymics = ["Иванович", "Петрович", "Алексеевич", "Дмитриевич", "Сергеевич", "Ивановна", "Петровна", "Алексеевна", "Дмитриевна", "Сергеевна"]

    patient_ids = [uid() for _ in range(N)]
    clinic_ids = [uid() for _ in range(N)]
    doctor_ids = [uid() for _ in range(N)]
    technician_ids = [uid() for _ in range(N)]
    equipment_ids = [uid() for _ in range(N)]
    image_type_ids = [uid() for _ in range(N)]
    medical_record_ids = [uid() for _ in range(N)]
    image_ids = [uid() for _ in range(N)]
    diagnosis_ids = [uid() for _ in range(N)]

    patients = []
    clinics = []
    doctors = []
    doctor_clinics = []
    technicians = []
    equipment = []
    image_types = []
    medical_records = []
    images = []
    diagnoses = []

    for i in range(N):
        full_name = f"{random.choice(last_names)} {random.choice(first_names)} {random.choice(patronymics)}"
        patients.append((q(patient_ids[i]), q(full_name), q(random_date())))
        clinics.append((q(clinic_ids[i]), q(f"Клиника #{i + 1}, Москва, улица Стоматологическая, дом {i + 1}")))
        doctors.append((q(doctor_ids[i]), q(random.choice(specialties))))
        doctor_clinics.append((q(doctor_ids[i]), q(clinic_ids[i])))
        tech_name = f"{random.choice(last_names)} {random.choice(first_names)} {random.choice(patronymics)}"
        technicians.append((q(technician_ids[i]), q(tech_name), q(clinic_ids[i])))
        equipment.append((q(equipment_ids[i]), q(f"Dental X-Ray Unit Model {i + 1}"), q(clinic_ids[i])))
        image_types.append((q(image_type_ids[i]), q(f"Image type #{i + 1}")))
        created_at = random_timestamp()
        updated_at = random_timestamp()
        medical_records.append((q(medical_record_ids[i]), q(patient_ids[i]), q(created_at), q(updated_at), "TRUE", q(f"Медицинская карта пациента #{i + 1}")))
        images.append((q(image_ids[i]), q(bytea_hex(i + 1)) + "::bytea", q(random_timestamp()), q(patient_ids[i]), q(technician_ids[i]), q(equipment_ids[i]), q(image_type_ids[i])))
        diagnoses.append((q(diagnosis_ids[i]), q(f"Тестовое заключение по стоматологическому снимку #{i + 1}"), q(random_timestamp()), q(image_ids[i]), q(doctor_ids[i]), q(medical_record_ids[i])))

    with open(OUTPUT_PATH, "w", encoding="utf-8") as file:
        file.write("-- Auto-generated DentoBase seed data\n")
        file.write("-- Contains 1000 rows for each project table\n\n")
        file.write("BEGIN;\n\n")
        file.write("TRUNCATE TABLE diagnoses, images, medical_records, doctor_clinics, doctors, technicians, equipment, image_types, patients, clinics RESTART IDENTITY CASCADE;\n\n")
        write_insert(file, "patients", ["id", "full_name", "birth_date"], patients)
        write_insert(file, "clinics", ["id", "address"], clinics)
        write_insert(file, "doctors", ["id", "specialty"], doctors)
        write_insert(file, "doctor_clinics", ["doctor_id", "clinic_id"], doctor_clinics)
        write_insert(file, "technicians", ["id", "full_name", "clinic_id"], technicians)
        write_insert(file, "equipment", ["id", "model", "clinic_id"], equipment)
        write_insert(file, "image_types", ["id", "name"], image_types)
        write_insert(file, "medical_records", ["id", "patient_id", "created_at", "updated_at", "is_active", "notes"], medical_records)
        write_insert(file, "images", ["id", "image_data", "upload_date", "patient_id", "tech_id", "equip_id", "type_id"], images)
        write_insert(file, "diagnoses", ["id", "text", "created_at", "image_id", "doctor_id", "medical_record_id"], diagnoses)
        file.write("COMMIT;\n\n")
        for table in ["patients", "clinics", "doctors", "doctor_clinics", "technicians", "equipment", "image_types", "medical_records", "images", "diagnoses"]:
            file.write(f"SELECT '{table}' AS table_name, COUNT(*) FROM {table};\n")

    print(f"Seed file generated: {OUTPUT_PATH}")
    print(f"Rows per table: {N}")


if __name__ == "__main__":
    main()
