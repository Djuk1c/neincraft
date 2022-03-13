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
#include "cubeVertexData.h"
#include "World.h"
#include "Chunk.h"

// Constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Forward delcarations
void calculateDeltaTime(double time);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(char const *filename);
void checkFPS();

// Deltatime
int nbFrames = 0;
double lastTime = glfwGetTime();
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Camera
Camera camera(glm::vec3(0.0f, 10.0f, 10.0f));
float lastX = (float)SCREEN_WIDTH/2, lastY = (float)SCREEN_HEIGHT/2;
bool firstMouse = true;

int main()
{
	Window mainWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
	GLFWwindow* window = mainWindow.window;

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    // Create default shader
    unsigned int shaderProgram = LoadShader("shaders/default.vert", "shaders/default.frag");

    // Creating our mesh from the chunk data
	srand(time(NULL));
    World world;
    world.generateWorld(camera.Position);

    // Load and generate textures
    unsigned int texAtlas = loadTexture("textures/atlas.png");

	// Matrixes
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texAtlas);

    glUniform2fv(uAtlasTexCoord, 1, glm::value_ptr(glm::vec2(1.0, 0.0)));

    // Render Loop
    while (!glfwWindowShouldClose(window))
    {
		// TODO: Frustum culling?

        glUseProgram(shaderProgram);
        checkFPS();
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
			// 6 vertices / face (should probably implement EBO, dis waste of memory (good enough!))
			glDrawArrays(GL_TRIANGLES, 0, it->second->facesCount*6);
			it++;
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void checkFPS()
{
    double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 2.0 )
    {
        std::cout << nbFrames / 2 << std::endl;
        nbFrames = 0;
        lastTime += 2.0;
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

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
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
