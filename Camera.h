#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
	UP,
	DOWN
};

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f,
        float pitch = 0.0f)
        : front(glm::vec3(0.0f, 0.0f, -1.0f)),
        movementSpeed(50.0f),
        mouseSensitivity(0.1f),
        zoom(45.0f) {
        this->position = position;
        worldUp = up;
        this->yaw = yaw;
        this->pitch = pitch;
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix.
    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    // Processes input received from keyboard.
    // Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems).
    // deltaTime is used to ensure consistent movement speed regardless of frame rate.
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        if (direction == FORWARD)
            position += front * velocity;
        if (direction == BACKWARD)
            position -= front * velocity;
        if (direction == LEFT)
            position -= right * velocity;
        if (direction == RIGHT)
            position += right * velocity;
		if (direction == UP)
			position += worldUp * velocity;
		if (direction == DOWN)
			position -= worldUp * velocity;
    }

    // Processes input received from a mouse input system.
    // Expects the offset value in both the x and y direction.
    // (Use this if you want to enable mouse look.)
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Make sure that when pitch is out of bounds, the screen doesn't get flipped.
        if (constrainPitch) {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        // Update front, right and up Vectors using the updated Euler angles.
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event.
    // Only requires input on the vertical wheel-axis.
    void ProcessMouseScroll(float yoffset) {
        zoom -= yoffset;
        if (zoom < 1.0f)
            zoom = 1.0f;
        if (zoom > 45.0f)
            zoom = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles.
    void updateCameraVectors() {
        glm::vec3 f;
        f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        f.y = sin(glm::radians(pitch));
        f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(f);
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};