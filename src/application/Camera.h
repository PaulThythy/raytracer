#ifndef CAMERA_H
#define CAMERA_H

#include "math/Ray.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
    struct UniformBufferObject {
        alignas(16) glm::mat4 m_model;
        alignas(16) glm::mat4 m_view;
        alignas(16) glm::mat4 m_proj;
    };


    Camera(
        const glm::mat4& view = glm::lookAt(
            glm::vec3(0.0, 0.0, 3.0),                                       //camera position
            glm::vec3(0, 0, 0),                                             //look at
            glm::vec3(0, 1, 0)                                              //up
        ),
        const glm::mat4& model, 
        const glm::mat4& proj
    )
        : m_model(model), m_view(view), m_proj(proj) {}

    ~Camera() = default;

    void setModel(const glm::mat4& model) { m_model = model; }
    void setView(const glm::mat4& view) { m_view = view; }
    void setProj(const glm::mat4& proj) { m_proj = proj; }

    UniformBufferObject getUniformBufferObject() const {
        UniformBufferObject ubo{};
        ubo.m_model = m_model;
        ubo.m_view = m_view;
        ubo.m_proj = m_proj;
        return ubo;
    }

private:
    glm::mat4 m_model;
    glm::mat4 m_view;
    glm::mat4 m_proj;
};

#endif // CAMERA_H