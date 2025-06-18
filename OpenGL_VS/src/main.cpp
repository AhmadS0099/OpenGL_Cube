#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "Globals.h"
#include "Ball.h"
#include "Utils.h"

// Globale besturingsvariabelen
bool isRunning = true;
bool resetRequested = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            isRunning = !isRunning; // Start/Stop
        }
        if (key == GLFW_KEY_R) {
            resetRequested = true;  // Reset
        }
    }
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Bouncing Balls", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback); // Key input toevoegen

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform vec2 offset;
        out vec2 FragPos;
        void main() {
            FragPos = aPos;
            gl_Position = vec4(aPos + offset, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 FragPos;
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            float dist = length(FragPos * 30.0);
            float intensity = 1.0 - dist;
            intensity = clamp(intensity, 0.0, 1.0);
            FragColor = vec4(color * intensity, 1.0);
        }
    )";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Cirkel voor de rand
    std::vector<float> circleVertices = generateCircleVertices(CIRCLE_RADIUS, CIRCLE_SEGMENTS);
    GLuint circleVAO, circleVBO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Ballen
    std::vector<float> ballVertices = generateCircleVertices(BALL_RADIUS, CIRCLE_SEGMENTS);
    GLuint ballVAO, ballVBO;
    glGenVertexArrays(1, &ballVAO);
    glGenBuffers(1, &ballVBO);
    glBindVertexArray(ballVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
    glBufferData(GL_ARRAY_BUFFER, ballVertices.size() * sizeof(float), ballVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

    std::vector<Ball> balls = { { 0.0f, 0.0f, INITIAL_SPEED, INITIAL_SPEED } };

    glUseProgram(shaderProgram);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Reset?
        if (resetRequested) {
            balls = { { 0.0f, 0.0f, INITIAL_SPEED, INITIAL_SPEED } };
            resetRequested = false;
        }

        // Update ballen
        if (isRunning) {
            std::vector<Ball> newBalls;

            for (auto& ball : balls) {
                ball.x += ball.vx;
                ball.y += ball.vy;

                float dist = std::sqrt(ball.x * ball.x + ball.y * ball.y);
                if (dist + BALL_RADIUS >= CIRCLE_RADIUS) {
                    float nx = ball.x / dist;
                    float ny = ball.y / dist;
                    float dot = ball.vx * nx + ball.vy * ny;
                    ball.vx -= 2 * dot * nx;
                    ball.vy -= 2 * dot * ny;
                    ball.x -= nx * 0.001f;
                    ball.y -= ny * 0.001f;

                    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
                    newBalls.push_back({
                        0.0f, 0.0f,
                        INITIAL_SPEED * cos(angle),
                        INITIAL_SPEED * sin(angle)
                    });
                }
            }

            for (size_t i = 0; i < balls.size(); ++i) {
                for (size_t j = i + 1; j < balls.size(); ++j) {
                    float dx = balls[j].x - balls[i].x;
                    float dy = balls[j].y - balls[i].y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist < 2 * BALL_RADIUS) {
                        resolveBallCollision(balls[i], balls[j]);
                    }
                }
            }

            balls.insert(balls.end(), newBalls.begin(), newBalls.end());
        }

        // Ballen tekenen
        glBindVertexArray(circleVAO);
        glUniform2f(offsetLoc, 0.0f, 0.0f);
        glUniform3f(colorLoc, 0.2f, 0.4f, 0.7f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);

        glBindVertexArray(ballVAO);
        for (size_t i = 0; i < balls.size(); ++i) {
            const auto& ball = balls[i];
            if (i % 10 == 0)
                glUniform3f(colorLoc, 0.2f, 1.0f, 0.5f);
            else
                glUniform3f(colorLoc, 1.0f, 0.5f, 0.2f);
            glUniform2f(offsetLoc, ball.x, ball.y);
            glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);
        }

        // Print het aantal ballen in de console
        std::cout << "Ball count: " << balls.size() << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
