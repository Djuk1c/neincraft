#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLFWwindow* setupGLFW()
{
    // Initujemo glfw i setamo 3.3gl verziju
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Pravimo window
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Learnin", NULL, NULL); // glfwGetPrimaryMonitor()
    if (window == NULL)
    {
        std::cout << "Failed to create new window.\n";
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return NULL;
	}

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    return window;
}
