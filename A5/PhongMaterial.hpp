#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"

class PhongMaterial : public Material {
public:
  PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess, double reflection, double refraction, int texture, double bump);
  virtual ~PhongMaterial();
  glm::vec3 getkd();
  glm::vec3 getks();
  double getShininess();
  double getReflection();
  double getRefraction();
  double getBump();
  int getTexture();
  void setkd(glm::vec3 kd);
  void setks(glm::vec3 ks);

private:
  glm::vec3 m_kd;
  glm::vec3 m_ks;
  double m_reflection;
  double m_shininess;
  double m_refraction;
  int m_texture;
  double m_bump;
};
