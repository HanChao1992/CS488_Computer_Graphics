#include "PhongMaterial.hpp"

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess, double reflection, double refraction, int texture, double bump)
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
	, m_reflection(reflection)
	, m_refraction(refraction)
	, m_texture(texture)
	, m_bump(bump)
{}

PhongMaterial::~PhongMaterial()
{}

glm::vec3 PhongMaterial::getkd() {
	return m_kd;
}

glm::vec3 PhongMaterial::getks() {
	return m_ks;
}

void PhongMaterial::setkd(glm::vec3 kd) {
	m_kd = kd;
}

void PhongMaterial::setks(glm::vec3 ks) {
	m_ks = ks;
}

int PhongMaterial::getTexture() {
	return m_texture;
}

double PhongMaterial::getBump() {
	return m_bump;
}

double PhongMaterial::getShininess() {
	return m_shininess;
}

double PhongMaterial::getReflection() {
	return m_reflection;
}

double PhongMaterial::getRefraction() {
	return m_refraction;
}
