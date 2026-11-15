#ifndef OBJ_2D_H
#define OBJ_2D_H
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"

class Camera2D {
public:
    glm::vec2 position;
    float zoom;

    Camera2D(float width, float height)
        : position(0.0f, 0.0f), zoom(1.0f) 
    {
        m_Width = width;
        m_Height = height;
        updateMatrices();
    }

    void updateMatrices() {
        float left   = position.x - (m_Width  / 2.0f) / zoom;
        float right  = position.x + (m_Width  / 2.0f) / zoom;
        float bottom = position.y + (m_Height / 2.0f) / zoom;
        float top    = position.y - (m_Height / 2.0f) / zoom;

        m_ViewProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    }

    void move(const glm::vec2& offset) { position += offset; updateMatrices(); }
    void setPosition(const glm::vec2& newPos) { position = newPos; updateMatrices(); }
    void setZoom(float newZoom) { zoom = newZoom; updateMatrices(); }

    glm::mat4 getViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

private:
    float m_Width;
    float m_Height;
    glm::mat4 m_ViewProjectionMatrix;
};

class Object2D {
public:
    float jumpCooldownTimer = 0.0f;
    bool isCircle;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec3 color;
    float mass;
    glm::vec2 velocity{0.0f, 0.0f};
    glm::vec2 acceleration{0.0f, 0.0f};
    bool isGrounded = false;
    glm::vec2 lastPlatformPos = position;
    glm::vec2 platformMovement = position - lastPlatformPos;
    bool heading = true; // TRUE = END || FALSE = START

    Object2D(float x, float y, float width, float height, bool isCircle, glm::vec3 color, float mass, float screenWidth, float screenHeight) {
        this->isCircle = isCircle;
        startPos = glm::vec2(x, y);
        this->position = glm::vec2(x, y);
        lastPlatformPos = position;
        platformMovement = position - lastPlatformPos;
        this->size = glm::vec2(width, height);
        this->color = color;
        this->mass = mass;
        screenWidth = screenWidth;
        screenHeight = screenHeight;
        float xSigns[4] = { -1.0f, -1.0f,  1.0f,  1.0f };
        float ySigns[4] = { -1.0f,  1.0f,  1.0f, -1.0f };
        float* vertices = new float[24]; // 4 vertices * (3 position + 3 color)
        for (int i = 0; i < 4; i++) {
            int idx = i * 6; // Jump by 6 floats per vertex
            vertices[idx + 0] = xSigns[i] * width/2;  // X position
            vertices[idx + 1] = ySigns[i] * height/2;  // Y position
            vertices[idx + 2] = 0.0f;           // Z position
            vertices[idx + 3] = color.r;        // Red
            vertices[idx + 4] = color.g;        // Green
            vertices[idx + 5] = color.b;        // Blue
        }
        unsigned int* indices = new unsigned int[6] {
            0, 1, 2,   // first triangle
            2, 3, 0    // second triangle
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void Draw(Shader &shader) {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
        shader.setMat4("model", model);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void Physics(float deltaTime, float friction) {
        // if(position.y <= -0.95) {isGrounded = true;}

        if(jumpCooldownTimer > 0.0f) { jumpCooldownTimer -= deltaTime;}

        if(isGrounded) {velocity.x -= velocity.x * friction * deltaTime;}

        if(!isGrounded) {glm::vec2 gravity(0.0f, 1400.0f); acceleration += gravity;}

        // Update position based on velocity and acceleration
        velocity += acceleration * deltaTime;
        position += velocity * deltaTime;

        isGrounded = false;
    }

    void processInput(GLFWwindow *window, Camera2D &camera) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        this->acceleration = glm::vec2(0.0f, 0.0f); // Reset acceleration each frame

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && this->jumpCooldownTimer <= 0.0f && isGrounded == true) { this->velocity.y = -500.0f; this->jumpCooldownTimer = 0.45f; isGrounded = false;}
        // if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) { this->velocity.y = -300.0f;}
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) { this->velocity.x = -150.0f;}
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { this->velocity.x = 150.0f;}

        camera.setPosition(position);
    }

    void checkCollision(Object2D &other, bool moving) {
        inCollision = false;
        float thisHalfW = size.x / 2.0f;
        float thisHalfH = size.y / 2.0f;
        float otherHalfW = other.size.x / 2.0f;
        float otherHalfH = other.size.y / 2.0f;

        float thisRight = position.x + thisHalfW;
        float thisLeft = position.x - thisHalfW;
        float thisTop = position.y - thisHalfH;
        float thisBottom = position.y + thisHalfH;

        float otherRight = other.position.x + otherHalfW;
        float otherLeft = other.position.x - otherHalfW;
        float otherTop = other.position.y - otherHalfH;
        float otherBottom = other.position.y + otherHalfH;

        float snapMargin = (moving && velocity.y >= 0.0f) ? 4.0f : 0.0f;
        bool collisionX = thisRight >= otherLeft && thisLeft <= otherRight;
        bool collisionY = (thisBottom + snapMargin) >= otherTop && thisTop <= otherBottom;

        if (collisionX && collisionY) {
            inCollision = true;
            // Find out how deep the overlap is on each axis
            float overlapX = std::min(thisRight - otherLeft, otherRight - thisLeft);
            float overlapY = std::min(thisBottom - otherTop, otherBottom - thisTop);

            // Whichever overlap is smaller is the side we hit from!
            if (overlapX < overlapY) {
                // Horizontal collision (walls)
                if (position.x < other.position.x) {
                    position.x = otherLeft - thisHalfW;
                    velocity.x = 0.0f;
                } else {
                    position.x = otherRight + thisHalfW;
                    velocity.x = 0.0f;
                }
            } else {
                // Vertical collision (floors and ceilings)
                if (position.y < other.position.y) {
                    if(moving) {
                        if (glm::distance(other.lastPlatformPos, other.position) > 5.0f) {other.lastPlatformPos = other.position;}
                        other.platformMovement = other.position - other.lastPlatformPos;
                        position += other.platformMovement; 
                        other.lastPlatformPos = other.position;
                    }

                    position.y = otherTop - thisHalfH;
                    velocity.y = 0.0f;
                    isGrounded = true;
                } else {
                    position.y = otherBottom + thisHalfH + (snapMargin >= 4.0f ? 1.0f : 0.0f);
                    velocity.y = 0.0f;
                }
            }
        }
    }

    void checkDamage(Object2D &other) {
        float thisHalfW = size.x / 2.0f;
        float thisHalfH = size.y / 2.0f;
        float otherHalfW = other.size.x / 2.0f;
        float otherHalfH = other.size.y / 2.0f;

        float thisRight = position.x + thisHalfW;
        float thisLeft = position.x - thisHalfW;
        float thisTop = position.y - thisHalfH;
        float thisBottom = position.y + thisHalfH;

        float otherRight = other.position.x + otherHalfW;
        float otherLeft = other.position.x - otherHalfW;
        float otherTop = other.position.y - otherHalfH;
        float otherBottom = other.position.y + otherHalfH;

        bool collisionX = thisRight >= otherLeft && thisLeft <= otherRight;
        bool collisionY = thisBottom >= otherTop && thisTop <= otherBottom;

        if (collisionX && collisionY) {
            position.x = startPos.x;
            position.y = startPos.y;
            velocity = glm::vec2(0.0f, 0.0f);
        }
    }

    void checkRespawn(Object2D &other) {
        float thisHalfW = size.x / 2.0f;
        float thisHalfH = size.y / 2.0f;
        float otherHalfW = other.size.x / 2.0f;
        float otherHalfH = other.size.y / 2.0f;

        float thisRight = position.x + thisHalfW;
        float thisLeft = position.x - thisHalfW;
        float thisTop = position.y - thisHalfH;
        float thisBottom = position.y + thisHalfH;

        float otherRight = other.position.x + otherHalfW;
        float otherLeft = other.position.x - otherHalfW;
        float otherTop = other.position.y - otherHalfH;
        float otherBottom = other.position.y + otherHalfH;

        bool collisionX = thisRight >= otherLeft && thisLeft <= otherRight;
        bool collisionY = thisBottom >= otherTop && thisTop <= otherBottom;

        if (collisionX && collisionY) {
            startPos = glm::vec2(other.position.x, otherTop);
        }
    }

    void checkJumpBoost(Object2D &other) {
        float thisHalfW = size.x / 2.0f;
        float thisHalfH = size.y / 2.0f;
        float otherHalfW = other.size.x / 2.0f;
        float otherHalfH = other.size.y / 2.0f;

        float thisRight = position.x + thisHalfW;
        float thisLeft = position.x - thisHalfW;
        float thisTop = position.y - thisHalfH;
        float thisBottom = position.y + thisHalfH;

        float otherRight = other.position.x + otherHalfW;
        float otherLeft = other.position.x - otherHalfW;
        float otherTop = other.position.y - otherHalfH;
        float otherBottom = other.position.y + otherHalfH;

        bool collisionX = thisRight >= otherLeft && thisLeft <= otherRight;
        bool collisionY = thisBottom >= otherTop && thisTop <= otherBottom;

        if(collisionX && collisionY) {
            velocity.y = -850.0f;
        }
    }

    bool collision(Object2D &other) {
        float thisHalfW = size.x / 2.0f;
        float thisHalfH = size.y / 2.0f;
        float otherHalfW = other.size.x / 2.0f;
        float otherHalfH = other.size.y / 2.0f;

        float thisRight = position.x + thisHalfW;
        float thisLeft = position.x - thisHalfW;
        float thisTop = position.y - thisHalfH;
        float thisBottom = position.y + thisHalfH;

        float otherRight = other.position.x + otherHalfW;
        float otherLeft = other.position.x - otherHalfW;
        float otherTop = other.position.y - otherHalfH;
        float otherBottom = other.position.y + otherHalfH;

        bool collisionX = thisRight >= otherLeft && thisLeft <= otherRight;
        bool collisionY = thisBottom >= otherTop && thisTop <= otherBottom;

        if(collisionX && collisionY) {
            return true;
        }
        return false;
    }

    void stepObject(glm::vec2 end, float step, float deltaTime) {
        static glm::vec2 start = startPos;
        glm::vec2 target = heading ? end : startPos;
        float distance = glm::distance(position, target);

        if(distance <= step * deltaTime) {
            position = target;
            heading = !heading;
            return;
        }

        glm::vec2 direction = glm::normalize(target - position);
        position += direction * step * deltaTime;
    }

    void setStartPos(glm::vec2 newStart) {
        startPos = newStart;
        position = newStart;
        velocity = glm::vec2(0.0f, 0.0f);
    }

private:
    unsigned int VAO, VBO, EBO;
    bool inCollision = false;
    glm::vec2 startPos = glm::vec2(0, 0);
};

class Level2D {
    public:
        std::vector<Object2D> walls;
        std::vector<Object2D> killBricks;
        std::vector<Object2D> jumpPads;
        std::vector<Object2D> respawnPoints;
        std::vector<Object2D> stepObjects;

        Level2D(Object2D& player, Shader& shader, float scrW, float scrH) : scrWidth(scrW), scrHeight(scrH), player(player), shader(shader) {}

        void wall(float x, float y, float width, float height) {walls.emplace_back(x, y, width, height, false, wallColor, 0.1f, scrWidth, scrHeight);}
        void killBrick(float x, float y, float width, float height) {killBricks.emplace_back(x, y, width, height, false, killColor, 0.1f, scrWidth, scrHeight);}
        void jumpPad(float x, float y, float width, float height) {jumpPads.emplace_back(x, y, width, height, false, jumpColor, 0.1f, scrWidth, scrHeight);}
        void respawnPoint(float x, float y, float width, float height) {respawnPoints.emplace_back(x, y, width, height, false, respawnColor, 0.1f, scrWidth, scrHeight);}
        void win(float x, float y, float width, float height) {winBlock = Object2D(x, y, width, height, false, winColor, 0.1f, scrWidth, scrHeight);}
        void step(float x, float y, float width, float height, glm::vec2 end, float step) {stepObjects.emplace_back(x, y, width, height, false, wallColor, 0.1f, scrWidth, scrHeight); stepInfo.emplace_back(glm::vec3(end, step));}

        void collisions(float& currentLevel, float deltaTime, glm::vec2 newSpawn) {
            for(int i = 0; i < stepInfo.size(); i++) {stepObjects[i].stepObject(glm::vec2(stepInfo[i].x, stepInfo[i].y), stepInfo[i].z, deltaTime);}
            for(auto& obj : walls) {player.checkCollision(obj, false);}
            for(auto& obj : killBricks) {player.checkCollision(obj, false); player.checkDamage(obj);}
            for(auto& obj : jumpPads) {player.checkCollision(obj, false); player.checkJumpBoost(obj);}
            for(auto& obj : respawnPoints) {player.checkCollision(obj, false); player.checkRespawn(obj);}
            for(auto& obj : stepObjects) {player.checkCollision(obj, true);}
            if(player.collision(winBlock)) {currentLevel++; player.setStartPos(newSpawn);}
        }

        void draw() {
            for(auto& obj : killBricks) {obj.Draw(shader);}
            for(auto& obj : walls) {obj.Draw(shader);}
            for(auto& obj : jumpPads) {obj.Draw(shader);}
            for(auto& obj : respawnPoints) {obj.Draw(shader);}
            for(auto& obj : stepObjects) {obj.Draw(shader);}
            winBlock.Draw(shader);
            player.Draw(shader);
        }

        void render(float& currentLevel, float deltaTime, glm::vec2 newSpawn) {
            collisions(currentLevel, deltaTime, newSpawn);
            draw();
        }

    private:
        float scrWidth;
        float scrHeight;
        glm::vec3 wallColor{0.33f, 0.37f, 0.43f};
        glm::vec3 killColor{0.8f, 0.1f, 0.1f};
        glm::vec3 respawnColor{0.21f, 0.96f, 0.0f};
        glm::vec3 winColor{1.0f, 0.91f, 0.01f};
        glm::vec3 jumpColor{0.98f, 0.72f, 0.01f};
        Object2D& player;
        Shader& shader;
        std::vector<glm::vec3> stepInfo;
        Object2D winBlock{0, 0, 0, 0, false, winColor, 0.1f, scrWidth, scrHeight};
};

#endif // OBJ_2D_H