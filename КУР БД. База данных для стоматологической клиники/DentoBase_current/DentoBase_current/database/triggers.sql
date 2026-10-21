CREATE OR REPLACE FUNCTION update_medical_record_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_update_medical_record_updated_at ON medical_records;

CREATE TRIGGER trg_update_medical_record_updated_at
BEFORE UPDATE ON medical_records
FOR EACH ROW
EXECUTE FUNCTION update_medical_record_updated_at();
