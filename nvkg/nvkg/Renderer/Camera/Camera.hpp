#ifndef NVKG_CAMERA_HPP
#define NVKG_CAMERA_HPP

//#include <nvkg/Renderer/Core.hpp>

/*
* Basic camera class
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <nvkg/Input/Input.hpp>

namespace nvkg {

    class CameraNew {
        private:
            float fov;
            float znear, zfar;

            void updateViewMatrix() {
                glm::mat4 rotM = glm::mat4(1.0f);
                glm::mat4 transM;

                rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
                rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

                glm::vec3 translation = position;
                if (flipY) {
                    translation.y *= -1.0f;
                }
                transM = glm::translate(glm::mat4(1.0f), translation);

                if (type == CameraType::firstperson) {
                    matrices.view = rotM * transM;
                } else {
                    matrices.view = transM * rotM;
                }

                viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

                updated = true;
            };

        public:

            enum CameraType { lookat, firstperson };
            CameraType type = CameraType::lookat;

            glm::vec3 rotation = glm::vec3();
            glm::vec3 position = glm::vec3();
            glm::vec4 viewPos = glm::vec4();

            float rotationSpeed = 1.0f;
            float movementSpeed = 1.0f;

            bool updated = false;
            bool flipY = false;

            struct {
                glm::mat4 perspective;
                glm::mat4 view;
            } matrices;

            struct {
                bool left = false;
                bool right = false;
                bool up = false;
                bool down = false;
            } keys;

            bool moving() {
                return Input::key_down(KEY_W) || Input::key_down(KEY_A) || Input::key_down(KEY_S) ||
                       Input::key_down(KEY_D) || Input::key_down(KEY_Q) || Input::key_down(KEY_E);
            }

            float getNearClip() { 
                return znear;
            }

            float getFarClip() {
                return zfar;
            }

            void setPerspective(float fov, float aspect, float znear, float zfar) {
                this->fov = fov;
                this->znear = znear;
                this->zfar = zfar;
                matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
                if (flipY) {
                    matrices.perspective[1][1] *= -1.0f;
                }
            };

            void updateAspectRatio(float aspect) {
                matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
                if (flipY) {
                    matrices.perspective[1][1] *= -1.0f;
                }
            }

            void setPosition(glm::vec3 position) {
                this->position = position;
                updateViewMatrix();
            }

            void setRotation(glm::vec3 rotation) {
                this->rotation = rotation;
                updateViewMatrix();
            }

            void rotate(glm::vec3 delta) {
                this->rotation += delta;
                updateViewMatrix();
            }

            void setTranslation(glm::vec3 translation) {
                this->position = translation;
                updateViewMatrix();
            };

            void translate(glm::vec3 delta) {
                this->position += delta;
                updateViewMatrix();
            }

            void setRotationSpeed(float rotationSpeed) {
                this->rotationSpeed = rotationSpeed;
            }

            void setMovementSpeed(float movementSpeed) {
                this->movementSpeed = movementSpeed;
            }

            void update(float deltaTime) {
                updated = false;
                if (type == CameraType::firstperson) {
                    if (moving()) {
                        glm::vec3 camFront;
                        camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
                        camFront.y = sin(glm::radians(rotation.x));
                        camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                        camFront = glm::normalize(camFront);

                        float moveSpeed = deltaTime * movementSpeed;

                        if (Input::key_down(KEY_W))
                            position += camFront * moveSpeed;
                        if (Input::key_down(KEY_S))
                            position -= camFront * moveSpeed;
                        if (Input::key_down(KEY_A))
                            position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
                        if (Input::key_down(KEY_D))
                            position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
                        if (Input::key_down(KEY_E))
                            position -= glm::normalize(glm::cross(camFront, glm::vec3(1.0f, 0.0f, 0.0f))) * moveSpeed;
                        if (Input::key_down(KEY_Q))
                            position += glm::normalize(glm::cross(camFront, glm::vec3(1.0f, 0.0f, 0.0f))) * moveSpeed;

                        updateViewMatrix();
                    }
                }
            };
    };
}

#endif