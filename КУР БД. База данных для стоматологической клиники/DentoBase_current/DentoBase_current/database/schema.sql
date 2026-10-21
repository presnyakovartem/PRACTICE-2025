CREATE EXTENSION IF NOT EXISTS pgcrypto;

DROP TABLE IF EXISTS diagnoses CASCADE;
DROP TABLE IF EXISTS images CASCADE;
DROP TABLE IF EXISTS medical_records CASCADE;
DROP TABLE IF EXISTS doctor_clinics CASCADE;
DROP TABLE IF EXISTS doctors CASCADE;
DROP TABLE IF EXISTS technicians CASCADE;
DROP TABLE IF EXISTS equipment CASCADE;
DROP TABLE IF EXISTS image_types CASCADE;
DROP TABLE IF EXISTS patients CASCADE;
DROP TABLE IF EXISTS clinics CASCADE;

CREATE TABLE patients (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    full_name TEXT NOT NULL,
    birth_date DATE NOT NULL
);

CREATE TABLE clinics (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    address TEXT NOT NULL
);

CREATE TABLE doctors (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    specialty TEXT NOT NULL CHECK (
        specialty IN (
            'THERAPIST',
            'ORTHODONTIST',
            'SURGEON',
            'PERIODONTIST',
            'ENDODONTIST',
            'IMPLANTOLOGIST',
            'PEDIATRIC_DENTIST'
        )
    )
);

CREATE TABLE doctor_clinics (
    doctor_id UUID NOT NULL REFERENCES doctors(id) ON DELETE CASCADE,
    clinic_id UUID NOT NULL REFERENCES clinics(id) ON DELETE CASCADE,
    PRIMARY KEY (doctor_id, clinic_id)
);

CREATE TABLE technicians (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    full_name TEXT NOT NULL,
    clinic_id UUID NOT NULL REFERENCES clinics(id) ON DELETE RESTRICT
);

CREATE TABLE equipment (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    model TEXT NOT NULL,
    clinic_id UUID NOT NULL REFERENCES clinics(id) ON DELETE RESTRICT
);

CREATE TABLE image_types (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE medical_records (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    patient_id UUID NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMP NOT NULL DEFAULT NOW(),
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    notes TEXT
);

CREATE TABLE images (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    image_data BYTEA NOT NULL,
    upload_date TIMESTAMP NOT NULL DEFAULT NOW(),
    patient_id UUID NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
    tech_id UUID NOT NULL REFERENCES technicians(id) ON DELETE RESTRICT,
    equip_id UUID NOT NULL REFERENCES equipment(id) ON DELETE RESTRICT,
    type_id UUID NOT NULL REFERENCES image_types(id) ON DELETE RESTRICT
);

CREATE TABLE diagnoses (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    text TEXT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    image_id UUID NOT NULL UNIQUE REFERENCES images(id) ON DELETE CASCADE,
    doctor_id UUID NOT NULL REFERENCES doctors(id) ON DELETE RESTRICT,
    medical_record_id UUID NOT NULL REFERENCES medical_records(id) ON DELETE CASCADE
);
