#include <iostream>
#include <time.h>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLShader.h"
#include "stb_image.h"
#include "camera.h"

#include "window.h"
#include "World.h"
#include "Chunk.h"

// Constants
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;

// Forward delcarations
glm::vec3 calculateRayCast();
void breakBlock();
void calculateDeltaTime(double time);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(char const *filename);
void checkFPS();

// Vars
int nbFrames = 0;
double lastTime = glfwGetTime();
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = (float)SCREEN_WIDTH/2, lastY = (float)SCREEN_HEIGHT/2;	//cam
bool firstMouse = true;

// Classes
Camera camera(glm::vec3(0.0f, 100.0f, 0.0f));
Window mainWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
World world;

// Matrixes
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

int main()
{
	GLFWwindow* window = mainWindow.window;

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Create default shader
    unsigned int shaderProgram = LoadShader("shaders/default.vert", "shaders/default.frag");

    // Creating our mesh from the chunk data
	srand(time(NULL));
    world.generateWorld(camera.Position);

    // Load and generate textures
    unsigned int texAtlas = loadTexture("textures/atlas.png");


    // Uniform Locations
    glUseProgram(shaderProgram);
    int uModel = glGetUniformLocation(shaderProgram, "model");
    int uProjection = glGetUniformLocation(shaderProgram, "projection");
    int uView = glGetUniformLocation(shaderProgram, "view");
    int uAtlasTexCoord = glGetUniformLocation(shaderProgram, "atlasTexCoord");

    // GL Sets
    glUseProgram(shaderProgram);
    glClearColor(0.3f,0.7f,0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texAtlas);

    glUniform2fv(uAtlasTexCoord, 1, glm::value_ptr(glm::vec2(1.0, 0.0)));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    // Render Loop
    while (!glfwWindowShouldClose(window))
    {
		// TODO: Frustum culling?

        glUseProgram(shaderProgram);
        //checkFPS();
        calculateDeltaTime(glfwGetTime());
        processInput(window);

        // Camera
        view = camera.GetViewMatrix();
		world.addNearbyChunks(camera.Position);

        // Update uniforms
        glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
		
		// Render world
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		std::map<std::pair<int, int>, Chunk*>::iterator it = world.worldChunks.begin();
		while (it != world.worldChunks.end())
		{
			glBindVertexArray(it->second->VAO);
			glDrawArrays(GL_TRIANGLES, 0, it->second->facesCount*6);
			it++;
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

//---------Functions---------//
// https://stackoverflow.com/questions/40276068/opengl-raycasting-with-any-object
// https://antongerdelan.net/opengl/raycasting.html
glm::vec3 calculateRayCast()
{
    glm::vec3 rayNds = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec4 rayClip = glm::vec4(rayNds, 1.0);
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x,rayEye.y, -1.0, 0.0);
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
    rayWorld = glm::normalize(rayWorld);
	return rayWorld;
}

void breakBlock()				// Doesnt work when breaking blocks nearing other chunks
{
	int maxDistance = 4;
	glm::vec3 ray = calculateRayCast();
	glm::vec3 start = camera.Position;
	for (int i = 0; i <= maxDistance; i++)
	{
		glm::vec3 scaledRay(ray.x*(float)i, ray.y*(float)i, ray.z*(float)i);
		glm::vec3 point = start + scaledRay;

		int xPos = (int)ceil(point.x / 16) - 1;
		int zPos = (int)ceil(point.z / 16) - 1;

		if (world.worldChunks.find(std::make_pair(xPos, zPos)) == world.worldChunks.end() || point.y > 64.0f)
			continue;

		int xBlock = abs((int)round(point.x) % 16);
		int yBlock = abs((int)round(point.y));
		int zBlock = abs((int)round(point.z) % 16);
		if (camera.Position.x < 0)
		{
			xBlock = abs(16-xBlock);
		}
		if (camera.Position.z < 0)
		{
			zBlock = abs(16-zBlock);
		}

		if (world.worldChunks.at(std::make_pair(xPos, zPos))->chunk[xBlock][yBlock][zBlock] == 1)
		{
			printf("Camera pos: %f, %f, %f\n", camera.Position.x, camera.Position.y, camera.Position.z);
			printf("Break %d, %d, %d\n", xBlock, yBlock, zBlock);
			world.worldChunks.at(std::make_pair(xPos, zPos))->chunk[xBlock][yBlock][zBlock] = 0;
			world.worldChunks.at(std::make_pair(xPos, zPos))->updateVBO();

			world.worldChunks.at(std::make_pair(xPos+1, zPos))->updateVBO();
			world.worldChunks.at(std::make_pair(xPos-1, zPos))->updateVBO();
			world.worldChunks.at(std::make_pair(xPos, zPos+1))->updateVBO();
			world.worldChunks.at(std::make_pair(xPos, zPos-1))->updateVBO();
			break;
		}
	}
}

void checkFPS()
{
    double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1.0 )
    {
        std::cout << nbFrames << std::endl;
        nbFrames = 0;
        lastTime += 1.0;
    }
}

unsigned int loadTexture(char const *filename)
{
    int width, height, nrChannels;
    unsigned int texture;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else
    {
        std::cout << "Failed to load texture, check path." <<  std::endl;
    }
    stbi_image_free(data);
    return texture;
}

void calculateDeltaTime(double time)
{
    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		breakBlock();
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		placeBlock();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
	
    // Camera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}
