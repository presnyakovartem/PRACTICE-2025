#include "ImageService.h"
#include <ctime>
#include <stdexcept>

ImageService::ImageService(std::shared_ptr<IImageRepository> imgRepo,
                           std::shared_ptr<IPatientRepository> patientRepo)
    : imageRepo(imgRepo), patientRepo(patientRepo) {}

Image ImageService::uploadImage(const std::vector<uint8_t>& imageData,
                                const uu_id& patientId,
                                const uu_id& techId,
                                const uu_id& equipId,
                                const uu_id& typeId) {
    if (imageData.empty()) throw std::invalid_argument("Image data is empty");
    patientRepo->getById(patientId); // проверяем существование пациента

    Image img;
    img.id = uu_id::generate();
    img.imageData = imageData;
    img.uploadDate = std::time(nullptr);
    img.patientId = patientId;
    img.techId = techId;
    img.equipId = equipId;
    img.typeId = typeId;

    imageRepo->add(img);
    return img;
}

std::string ImageService::getPatientName(const uu_id& patientId) {
    Patient p = patientRepo->getById(patientId);
    return p.fullName;
}