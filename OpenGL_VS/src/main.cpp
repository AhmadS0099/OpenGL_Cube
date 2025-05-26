#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const float CIRCLE_RADIUS = 0.7f;
const int CIRCLE_SEGMENTS = 100;
const float BALL_RADIUS = CIRCLE_RADIUS / 10.0f;

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform vec2 offset;
void main() {
    gl_Position = vec4(aPos + offset, 0.0, 1.0);
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

// Ball structure
struct Ball {
    float x, y;
    float vx, vy;
};

std::vector<float> generateCircleVertices(float radius, int segments) {
    std::vector<float> vertices;
    vertices.push_back(0.0f); // center
    vertices.push_back(0.0f);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(radius * cos(angle));
        vertices.push_back(radius * sin(angle));
    }
    return vertices;
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    // GLFW & OpenGL Init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Endless Bouncing Ball", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Circle VAO
    std::vector<float> circleVertices = generateCircleVertices(CIRCLE_RADIUS, CIRCLE_SEGMENTS);
    GLuint circleVAO, circleVBO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Ball VAO (smaller circle)
    std::vector<float> ballVertices = generateCircleVertices(BALL_RADIUS, CIRCLE_SEGMENTS);
    GLuint ballVAO, ballVBO;
    glGenVertexArrays(1, &ballVAO);
    glGenBuffers(1, &ballVBO);
    glBindVertexArray(ballVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
    glBufferData(GL_ARRAY_BUFFER, ballVertices.size() * sizeof(float), ballVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set uniforms
    GLint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

    // Initial ball 
    Ball ball = { 0.0f, 0.0f, 0.0004f, 0.0006f };

    // Enable blending
    glUseProgram(shaderProgram);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw outer circle
        glBindVertexArray(circleVAO);
        glUniform2f(offsetLoc, 0.0f, 0.0f);
        glUniform3f(colorLoc, 0.2f, 0.4f, 0.7f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);

        // Update ball position (no gravity)
        ball.x += ball.vx;
        ball.y += ball.vy;

        // Reflect from circular wall
        float dist = std::sqrt(ball.x * ball.x + ball.y * ball.y);
        if (dist + BALL_RADIUS >= CIRCLE_RADIUS) {
            float nx = ball.x / dist;
            float ny = ball.y / dist;
            float dot = ball.vx * nx + ball.vy * ny;
            ball.vx -= 2 * dot * nx;
            ball.vy -= 2 * dot * ny;

            // Push ball slightly inside to avoid sticking
            float overlap = (dist + BALL_RADIUS) - CIRCLE_RADIUS;
            ball.x -= overlap * nx;
            ball.y -= overlap * ny;
        }

        // Draw ball
        glBindVertexArray(ballVAO);
        glUniform2f(offsetLoc, ball.x, ball.y);
        glUniform3f(colorLoc, 1.0f, 0.5f, 0.2f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
