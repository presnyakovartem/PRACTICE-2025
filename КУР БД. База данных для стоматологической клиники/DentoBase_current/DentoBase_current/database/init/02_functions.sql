CREATE OR REPLACE FUNCTION get_patient_images(p_patient_id UUID)
RETURNS TABLE (
    image_id UUID,
    upload_date TIMESTAMP,
    image_type TEXT,
    equipment_model TEXT
) AS $$
BEGIN
    RETURN QUERY
    SELECT i.id, i.upload_date, it.name, e.model
    FROM images i
    JOIN image_types it ON i.type_id = it.id
    JOIN equipment e ON i.equip_id = e.id
    WHERE i.patient_id = p_patient_id
    ORDER BY i.upload_date DESC;
END;
$$ LANGUAGE plpgsql;
