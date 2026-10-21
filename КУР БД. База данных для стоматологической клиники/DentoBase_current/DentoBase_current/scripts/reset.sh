#!/bin/bash

set -e

CONTAINER_NAME="dentobase-postgres"
DATABASE_NAME="dentobase_test"
POSTGRES_USER="postgres"

echo "=========================================="
echo "DentoBase full database recreation started"
echo "=========================================="

if command -v docker-compose >/dev/null 2>&1; then
    COMPOSE_CMD="docker-compose"
elif docker compose version >/dev/null 2>&1; then
    COMPOSE_CMD="docker compose"
else
    echo "Error: docker-compose or docker compose is not available."
    exit 1
fi

echo "[1/8] Starting Docker containers..."
$COMPOSE_CMD up -d postgres redis

echo "[2/8] Waiting for PostgreSQL..."
for i in {1..30}; do
    if docker exec "$CONTAINER_NAME" pg_isready -U "$POSTGRES_USER" >/dev/null 2>&1; then
        echo "PostgreSQL is ready."
        break
    fi
    if [ "$i" -eq 30 ]; then
        echo "Error: PostgreSQL did not become ready in time."
        exit 1
    fi
    sleep 1
done

echo "[3/8] Copying SQL files to container..."
docker cp database/schema.sql "$CONTAINER_NAME":/tmp/schema.sql
docker cp database/functions.sql "$CONTAINER_NAME":/tmp/functions.sql
docker cp database/triggers.sql "$CONTAINER_NAME":/tmp/triggers.sql
docker cp database/roles.sql "$CONTAINER_NAME":/tmp/roles.sql

echo "[4/8] Recreating database $DATABASE_NAME..."
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d postgres -c "SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE datname = '$DATABASE_NAME';"
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d postgres -c "DROP DATABASE IF EXISTS $DATABASE_NAME;"
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d postgres -c "CREATE DATABASE $DATABASE_NAME;"

echo "[5/8] Applying schema.sql..."
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/schema.sql

echo "[6/8] Applying functions.sql..."
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/functions.sql

echo "[7/8] Applying triggers.sql..."
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/triggers.sql

echo "[8/8] Generating and applying seed data..."
python3 scripts/generate_seed.py
if [ ! -f database/seed.sql ]; then
    echo "Error: database/seed.sql was not generated."
    exit 1
fi
docker cp database/seed.sql "$CONTAINER_NAME":/tmp/seed.sql
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/seed.sql

echo "Applying roles.sql..."
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/roles.sql

echo "Checking row counts..."
docker exec -i "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -c "
SELECT 'patients' AS table_name, COUNT(*) FROM patients
UNION ALL SELECT 'clinics', COUNT(*) FROM clinics
UNION ALL SELECT 'doctors', COUNT(*) FROM doctors
UNION ALL SELECT 'doctor_clinics', COUNT(*) FROM doctor_clinics
UNION ALL SELECT 'technicians', COUNT(*) FROM technicians
UNION ALL SELECT 'equipment', COUNT(*) FROM equipment
UNION ALL SELECT 'image_types', COUNT(*) FROM image_types
UNION ALL SELECT 'medical_records', COUNT(*) FROM medical_records
UNION ALL SELECT 'images', COUNT(*) FROM images
UNION ALL SELECT 'diagnoses', COUNT(*) FROM diagnoses
ORDER BY table_name;
"

echo "=========================================="
echo "DentoBase full database recreation finished"
echo "=========================================="
