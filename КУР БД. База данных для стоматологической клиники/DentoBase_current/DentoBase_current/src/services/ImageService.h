#ifndef IMAGESERVICE_H
#define IMAGESERVICE_H

#include <memory>
#include <vector>
#include <cstdint>
#include "repositories/interfaces/IImageRepository.h"
#include "repositories/interfaces/IPatientRepository.h"
#include "domain/Image.h"

class ImageService {
public:
    ImageService(std::shared_ptr<IImageRepository> imgRepo,
                 std::shared_ptr<IPatientRepository> patientRepo);
    
    Image uploadImage(const std::vector<uint8_t>& imageData,
                      const uu_id& patientId,
                      const uu_id& techId,
                      const uu_id& equipId,
                      const uu_id& typeId);
    
    std::string getPatientName(const uu_id& patientId);

private:
    std::shared_ptr<IImageRepository> imageRepo;
    std::shared_ptr<IPatientRepository> patientRepo;
};

#endif // IMAGESERVICE_H