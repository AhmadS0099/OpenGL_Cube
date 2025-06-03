#pragma once

#include <vector>
#include <cmath>
#include "Globals.h"
#include "Ball.h"
#include <glad/glad.h>    
#include <GLFW/glfw3.h>   


std::vector<float> generateCircleVertices(float radius, int segments);

GLuint compileShader(GLenum type, const char* source);

void resolveBallCollision(Ball& a, Ball& b);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
