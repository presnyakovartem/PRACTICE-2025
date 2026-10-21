#!/bin/bash
set -e

CONTAINER_NAME="dentobase-postgres"
DATABASE_NAME="dentobase_test"
POSTGRES_USER="postgres"

echo "Applying DentoBase roles..."
docker cp database/roles.sql "$CONTAINER_NAME":/tmp/roles.sql
docker exec -it "$CONTAINER_NAME" psql -U "$POSTGRES_USER" -d "$DATABASE_NAME" -f /tmp/roles.sql
echo "Roles applied successfully."
