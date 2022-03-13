#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
	GLFWwindow* window;
	Window(int width, int height);
};
