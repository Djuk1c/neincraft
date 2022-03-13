#ifndef GLSHADER_H
#define GLSHADER_H

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLuint LoadShader(const char *vertex_path, const char *fragment_path);

#endif
