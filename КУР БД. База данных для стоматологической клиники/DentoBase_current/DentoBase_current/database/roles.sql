REVOKE ALL ON SCHEMA public FROM PUBLIC;

DO $$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'dentobase_admin') THEN
        CREATE ROLE dentobase_admin NOLOGIN;
    END IF;

    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'dentobase_doctor') THEN
        CREATE ROLE dentobase_doctor NOLOGIN;
    END IF;

    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'dentobase_technician') THEN
        CREATE ROLE dentobase_technician NOLOGIN;
    END IF;
END
$$;

DO $$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'dentobase_admin_user') THEN
        CREATE ROLE dentobase_admin_user LOGIN PASSWORD 'admin_password';
    ELSE
        ALTER ROLE dentobase_admin_user WITH PASSWORD 'admin_password';
    END IF;

    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'dentobase_doctor_user') THEN
        CREATE ROLE dentobase_doctor_user LOGIN PASSWORD 'doctor_password';
    ELSE
        ALTER ROLE dentobase_doctor_user WITH PASSWORD 'doctor_password';
    END IF;

    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'dentobase_technician_user') THEN
        CREATE ROLE dentobase_technician_user LOGIN PASSWORD 'technician_password';
    ELSE
        ALTER ROLE dentobase_technician_user WITH PASSWORD 'technician_password';
    END IF;
END
$$;

GRANT dentobase_admin TO dentobase_admin_user;
GRANT dentobase_doctor TO dentobase_doctor_user;
GRANT dentobase_technician TO dentobase_technician_user;

GRANT CONNECT ON DATABASE dentobase_test TO dentobase_admin;
GRANT CONNECT ON DATABASE dentobase_test TO dentobase_doctor;
GRANT CONNECT ON DATABASE dentobase_test TO dentobase_technician;

GRANT USAGE ON SCHEMA public TO dentobase_admin;
GRANT USAGE ON SCHEMA public TO dentobase_doctor;
GRANT USAGE ON SCHEMA public TO dentobase_technician;

GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO dentobase_admin;
GRANT EXECUTE ON ALL FUNCTIONS IN SCHEMA public TO dentobase_admin;

GRANT SELECT ON
    patients,
    clinics,
    doctors,
    doctor_clinics,
    technicians,
    equipment,
    image_types,
    images,
    medical_records,
    diagnoses
TO dentobase_doctor;

GRANT INSERT, UPDATE ON diagnoses TO dentobase_doctor;
GRANT EXECUTE ON FUNCTION get_patient_images(UUID) TO dentobase_doctor;

GRANT SELECT ON
    patients,
    clinics,
    doctors,
    doctor_clinics,
    technicians,
    equipment,
    image_types,
    images,
    medical_records
TO dentobase_technician;

GRANT INSERT, UPDATE ON
    patients,
    medical_records,
    images
TO dentobase_technician;

GRANT EXECUTE ON FUNCTION get_patient_images(UUID) TO dentobase_technician;
