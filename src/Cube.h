#pragma once
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Cube
{
	private:
		std::vector<float> data;	
		glm::ivec3 position;
		int VAO, VBO;
	public:
		Cube(glm::ivec3 pos);
		~Cube();
};
