#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera
{
	struct UniformBufferObject {
		alignas(16) glm::mat4 m_model;
		alignas(16) glm::mat4 m_view;
		alignas(16) glm::mat4 m_proj;
	};

	glm::vec3 m_position;
	glm::vec3 m_lookAt;
	glm::vec3 m_front;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_worldUp;

	float m_yaw;
	float m_pitch;
	float m_fov;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;

	Camera() {}

	Camera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 up, float yaw, float pitch, float fov, float aspectRatio, float nearPlane, float farPlane)
		: m_position(position), m_lookAt(lookAt), m_up(up), m_yaw(yaw), m_pitch(pitch), m_fov(fov), m_aspectRatio(aspectRatio), m_nearPlane(nearPlane), m_farPlane(farPlane) {
		m_worldUp = up;

		updateCameraVectors();
	}

	inline glm::mat4 getViewMatrix() const {
		return glm::lookAt(m_position, m_lookAt, m_up);
	}

	inline glm::mat4 getProjectionMatrix() const {
		glm::mat4 proj = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
		proj[1][1] *= -1; // correction for vulkan
		return proj;
	}

	inline glm::mat4 getModelMatrix() const {
		return glm::mat4(1.0f);
	}

	inline void updateCameraVectors() {
		glm::vec3 front;
		front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		front.y = sin(glm::radians(m_pitch));
		front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_front = glm::normalize(front);

		// Recalculer les vecteurs "right" et "up"
		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}

	inline void updateCameraUBO(UniformBufferObject& ubo, float deltaTime) {
		ubo.m_model = getModelMatrix();
		ubo.m_view = getViewMatrix();
		ubo.m_proj = getProjectionMatrix();
	}
};