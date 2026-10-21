
#include "scene.h"

Scene::Scene() : light(), camera() {}

Scene::~Scene() {}

void Scene::update(float deltaTime) {
  for (auto &system : particleSystems) {
    system.update(deltaTime);
  }
}

void Scene::addParticleSystem() { particleSystems.emplace_back(); }

void Scene::addParticleSystem(const Vector3 &position) {
  particleSystems.emplace_back(position);
}

void Scene::removeParticleSystem(int index) {
  if (index >= 0 && index < static_cast<int>(particleSystems.size())) {
    particleSystems.erase(particleSystems.begin() + index);
  }
}
