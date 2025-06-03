#include "Utils.h"
#include <iostream>

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
    // Optionally add error checking here
    return shader;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

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

    float overlap = (2 * BALL_RADIUS - dist) / 2.0f;
    a.x -= overlap * nx;
    a.y -= overlap * ny;
    b.x += overlap * nx;
    b.y += overlap * ny;
}
