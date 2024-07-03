#ifndef CAMERA_H
#define CAMERA_H

#include "math/Ray.h"

#include <glm/glm.hpp>

struct Camera {
	glm::vec3 m_position;
	glm::vec3 m_lookAt;
	glm::vec3 m_up;

	double m_fov;
	double m_aspectRatio;
	double m_aperture;
	double m_focusDist;

	double m_nearPlane;
	double m_farPlane;

	int m_screenWidth;
	int m_screenHeight;

	Camera(
		const glm::vec3& position, const glm::vec3& lookAt, const glm::vec3& up,
		double fov, double aspectRatio, double aperture, double focusDist,
		double nearPlane = 0.1, double farPlane = 300.0
	): m_position(position), m_lookAt(lookAt), m_up(up), m_fov(fov), m_aspectRatio(aspectRatio), m_aperture(aperture), m_focusDist(focusDist) {
		
		this->m_nearPlane = nearPlane; this->m_farPlane = farPlane;
	}

	Ray::Ray getRay(float u, float v) const;
};

#endif