#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include "shader.hpp"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.hpp"
#include "obj_2d.hpp"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void frame_buffer_size_callback(GLFWwindow* window, int width, int height);

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Window", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Camera2D camera(800.0f, 600.0f);

    Object2D myObject(-50.0f, 550.0f, 25.0f, 25.0f, false, glm::vec3(0.0f, 0.5f, 0.2f), 1.0f, 800.0f, 600.0f);
    Shader myShader("../src/shader2.vs", "../src/shader2.fs");

    float currentLevel = 1;
    Level2D level1(myObject, myShader, 800.0f, 600.0f);
    level1.wall(0.0f, 550.0f, 25.0f, 80.0f);
    level1.wall(75.0f, 515.0f, 25.0f, 155.0f);
    level1.wall(150.0f, 485.0f, 25.0f, 225.0f);
    level1.wall(300.0f, 350.0f, 175.0f, 25.0f);
    level1.wall(0.0f, 600.0f, 800.0f, 25.0f);
    level1.killBrick(0.0f, 1000.0f, 2000.0f, 25.0f);
    level1.killBrick(200.0f, 585.0f, 400.0f, 15.0f);
    level1.jumpPad(367.5f, 336.0f, 25.0f, 5.0f);
    level1.step(450.0f, 150.0f, 50.0f, 15.0f, glm::vec2(600.0f, 150.0f), 50.0f);
    level1.win(700.0f, 150.0f, 50.0f, 15.0f);

    Level2D level2(myObject, myShader, 800.0f, 600.0f);
    level2.wall(0.0f, 600.0f, 800.0f, 25.0f);
    level2.step(75.0f, 550.0f, 50.0f, 15.0f, glm::vec2(75.0f, 450.0f), 50.0f);
    level2.step(150.0f, 400.0f, 50.0f, 15.0f, glm::vec2(150.0f, 300.0f), 100.0f);
    level2.wall(300.0f, 300.0f, 250.0f, 25.0f);
    level2.killBrick(325.0f, 250.0f, 25.0f, 50.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    float lastFrame = static_cast<float>(glfwGetTime());
    while(!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

    // Render

        myShader.use();

        // Input
        myObject.processInput(window, camera);

        // Physics
        myObject.Physics(deltaTime, 50.0f);

        // Clear
        glClearColor(0.92f, 0.8f, 0.79f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update Camera
        camera.updateMatrices();
        glm::mat4 vp = camera.getViewProjectionMatrix();
        myShader.setMat4("viewproj", vp);

        if(currentLevel == 1) {level1.render(currentLevel, deltaTime, glm::vec2(-50.0f, 550.0f));}
        else if(currentLevel == 2) {level2.render(currentLevel, deltaTime, glm::vec2(-50.0f, 550.0f));}

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}