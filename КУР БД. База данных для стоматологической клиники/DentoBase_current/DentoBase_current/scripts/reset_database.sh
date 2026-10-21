#!/bin/bash
set -e

CONTAINER_NAME="dentobase-postgres"
DATABASE_NAME="dentobase_test"
POSTGRES_USER="postgres"

echo "Resetting database ${DATABASE_NAME}..."

docker exec -it "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d postgres -c \
"SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE datname = '${DATABASE_NAME}';"

docker exec -it "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d postgres -c "DROP DATABASE IF EXISTS ${DATABASE_NAME};"
docker exec -it "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d postgres -c "CREATE DATABASE ${DATABASE_NAME};"

docker cp database/schema.sql "$CONTAINER_NAME":/tmp/schema.sql
docker cp database/functions.sql "$CONTAINER_NAME":/tmp/functions.sql
docker cp database/triggers.sql "$CONTAINER_NAME":/tmp/triggers.sql
docker cp database/roles.sql "$CONTAINER_NAME":/tmp/roles.sql

docker exec -it "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/schema.sql
docker exec -it "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/functions.sql
docker exec -it "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/triggers.sql
docker exec -it "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/roles.sql

echo "Database reset completed."
