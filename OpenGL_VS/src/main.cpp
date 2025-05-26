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
const float BALL_RADIUS = CIRCLE_RADIUS / 40.0f;
const float INITIAL_SPEED = 0.0004f;

struct Ball {
    float x, y;
    float vx, vy;
};

std::vector<float> generateCircleVertices(float radius, int segments) {
    std::vector<float> vertices;
    vertices.push_back(0.0f);
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

// Simple elastic collision between 2 balls
void resolveBallCollision(Ball& a, Ball& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist == 0.0f) return;

    float nx = dx / dist;
    float ny = dy / dist;

    float dvx = b.vx - a.vx;
    float dvy = b.vy - a.vy;
    float dot = dvx * nx + dvy * ny;

    if (dot > 0) return;

    float impulse = 2.0f * dot / 2.0f; // equal mass
    a.vx += impulse * nx;
    a.vy += impulse * ny;
    b.vx -= impulse * nx;
    b.vy -= impulse * ny;

    // Optional: push them apart to avoid overlap
    float overlap = (2 * BALL_RADIUS - dist) / 2.0f;
    a.x -= overlap * nx;
    a.y -= overlap * ny;
    b.x += overlap * nx;
    b.y += overlap * ny;
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Cloning Bouncing Balls", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform vec2 offset;
    void main() {
        gl_Position = vec4(aPos + offset, 0.0, 1.0);
    })";

    const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    })";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::vector<float> circleVertices = generateCircleVertices(CIRCLE_RADIUS, CIRCLE_SEGMENTS);
    GLuint circleVAO, circleVBO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

        // Draw outer circle
        glBindVertexArray(circleVAO);
        glUniform2f(offsetLoc, 0.0f, 0.0f);
        glUniform3f(colorLoc, 0.2f, 0.4f, 0.7f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);

        std::vector<Ball> newBalls;

        // Update ball positions and handle wall collisions
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

                // Spawn a new ball at center with small random direction
                float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
                newBalls.push_back({
                    0.0f, 0.0f,
                    // all.x, ball.y, spawn from the collision side, but it wont work cause it crashes
                    INITIAL_SPEED * cos(angle),
                    INITIAL_SPEED * sin(angle)
                    });
            }
        }

        // Handle ball-ball collisions
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

        // Append new balls
        balls.insert(balls.end(), newBalls.begin(), newBalls.end());

        // count the balls
        std::cout << "Ball count: " << balls.size() << std::endl;


        // Draw balls
        glBindVertexArray(ballVAO);
        for (const auto& ball : balls) {
            glUniform2f(offsetLoc, ball.x, ball.y);
            glUniform3f(colorLoc, 1.0f, 0.5f, 0.2f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
