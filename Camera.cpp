#include "Camera.hpp"
#include <glm/glm.hpp>
namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        
        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUpDirection, cameraFrontDirection));
        this->cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);
        rotate(0.0f, 0.0f);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 displacement = glm::vec3(0);

        switch (direction) {
        case MOVE_RIGHT:
            displacement += glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
            break;
        case MOVE_LEFT:
            displacement -= glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
            break;
        case MOVE_FORWARD:
            displacement += speed * cameraFrontDirection;
            break;
        case MOVE_BACKWARD:
            displacement -= speed * cameraFrontDirection;
            break;
        case MOVE_UP:
            displacement += glm::normalize(cameraUpDirection) * speed;
            break;
        case MOVE_DOWN:
            displacement -= glm::normalize(cameraUpDirection) * speed;
            break;
        default:
            break;
        }

        cameraPosition += displacement;
        cameraTarget += displacement;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float yaw, float pitch) {

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        );

        cameraTarget = cameraPosition + direction;
        cameraFrontDirection = glm::normalize(direction);
    }

    glm::vec3 Camera::getCameraPosition()
    {
        return cameraPosition;
    }
    glm::vec3 Camera::getCameraTarget()
    {
        return cameraTarget;
    }
    void Camera::setCameraPosition(glm::vec3 pozition)
    {
        cameraPosition = pozition;
    }
    void Camera::setCameraTarget(glm::vec3 target)
    {
        cameraTarget = target;
    }
}
