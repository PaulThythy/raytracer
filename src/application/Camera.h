#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera
{
	struct UniformBufferObject {
		alignas(16) glm::vec3 m_position;
		alignas(16) glm::vec3 m_lookAt;
		alignas(16) glm::vec3 m_front;
		alignas(16) glm::vec3 m_up;
		alignas(16) glm::vec3 m_right;
		alignas(16) glm::vec3 m_worldUp;
		
		alignas(4) float m_fov;
		alignas(4) float m_aspectRatio;
		alignas(4) float m_nearPlane;
		alignas(4) float m_farPlane;
	};

	UniformBufferObject m_cameraUBO;

	Camera() {}

	Camera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 up, float fov, float aspectRatio, float nearPlane, float farPlane) {
		m_cameraUBO.m_position = position;
		m_cameraUBO.m_lookAt = lookAt;
		m_cameraUBO.m_up = up;
		m_cameraUBO.m_fov = fov;
		m_cameraUBO.m_aspectRatio = aspectRatio;
		m_cameraUBO.m_nearPlane = nearPlane;
		m_cameraUBO.m_farPlane = farPlane;
		m_cameraUBO.m_worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
		updateCameraVectors();
	}

	inline void updateCameraVectors() {
		m_cameraUBO.m_front = glm::normalize(m_cameraUBO.m_lookAt - m_cameraUBO.m_position);

		m_cameraUBO.m_right = glm::normalize(glm::cross(m_cameraUBO.m_front, m_cameraUBO.m_worldUp));
		m_cameraUBO.m_up = glm::normalize(glm::cross(m_cameraUBO.m_right, m_cameraUBO.m_front));
	}

	inline void updateCameraUBO(UniformBufferObject& ubo, float deltaTime) {
		ubo.m_position = m_cameraUBO.m_position;
		ubo.m_lookAt = m_cameraUBO.m_lookAt;
		ubo.m_front = m_cameraUBO.m_front;
		ubo.m_up = m_cameraUBO.m_up;
		ubo.m_right = m_cameraUBO.m_right;
		ubo.m_worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
		ubo.m_aspectRatio = m_cameraUBO.m_aspectRatio;
		ubo.m_fov = m_cameraUBO.m_fov;
		ubo.m_nearPlane = m_cameraUBO.m_nearPlane;
		ubo.m_farPlane = m_cameraUBO.m_farPlane;
	}

	inline void moveForward(float distance) {
        glm::vec3 direction = glm::normalize(m_cameraUBO.m_front);
        m_cameraUBO.m_position += direction * distance;
        m_cameraUBO.m_lookAt += direction * distance;
        updateCameraVectors();
    }

	inline void moveRight(float distance) {
        glm::vec3 direction = glm::normalize(m_cameraUBO.m_right);
        m_cameraUBO.m_position += direction * distance;
        m_cameraUBO.m_lookAt += direction * distance;
        updateCameraVectors();
    }

	inline void moveUp(float distance) {
        glm::vec3 direction = glm::normalize(m_cameraUBO.m_up);
        m_cameraUBO.m_position += direction * distance;
        m_cameraUBO.m_lookAt += direction * distance;
        updateCameraVectors();
    }

	inline void rotateAroundUp(float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), m_cameraUBO.m_worldUp);
        glm::vec3 direction = m_cameraUBO.m_lookAt - m_cameraUBO.m_position;
        direction = glm::vec3(rotation * glm::vec4(direction, 0.0f));
        m_cameraUBO.m_lookAt = m_cameraUBO.m_position + direction;
        updateCameraVectors();
    }

    inline void rotateAroundRight(float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), m_cameraUBO.m_right);
        glm::vec3 direction = m_cameraUBO.m_lookAt - m_cameraUBO.m_position;
        direction = glm::vec3(rotation * glm::vec4(direction, 0.0f));
        m_cameraUBO.m_lookAt = m_cameraUBO.m_position + direction;
        updateCameraVectors();
    }
};