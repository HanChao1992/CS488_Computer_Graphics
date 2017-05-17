#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"


int colorClose(std::vector<glm::vec3> colorVec);

glm::vec3 worldMapping (
	glm::vec3 pixel, 
	const glm::vec3 eye, 
	const glm::vec3 view, 
	double fovy, 
	size_t w, 
	size_t h, 
	double focal
);

glm::vec3 lightRayColor(Intersection inter,
						SceneNode * root,
						const std::list<Light *> & lights,
						const glm::vec3 & hP,
						const glm::vec3 & ambient,
						float x,
						float y,
						size_t w,
						size_t h
);

glm::vec3 rayTrace(
	const glm::vec3 & eye,
	const glm::vec3 & worldC,
	SceneNode * root,
	const std::list<Light *> & lights,
	uint x,
	uint y,
	size_t w,
	size_t h,
	const glm::vec3 & ambient
);


glm::vec3 adaptiveSampling(
	float fac,
	int counter,
	const glm::vec3 & eye,
	const glm::vec3 & view,
	double fovy,
	SceneNode * root,
	const std::list<Light *> & lights,
	float x,
	float y,
	size_t w,
	size_t h,
	const glm::vec3 & ambient
);

void A5_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,
		double focal,
		int samp,
		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights,
		const std::vector<std::string> & textureLib
);
