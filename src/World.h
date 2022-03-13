#pragma once
#include <map>
#include <math.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Chunk.h"

class World
{
    public:
		const int CHUNKS_NUM = 6;
		std::map<std::pair<int, int>, Chunk*> worldChunks;
        void generateWorld(glm::vec3 cameraPos);
		bool addChunkAtLocation(int xPos, int zPos);
		void addNearbyChunks(glm::vec3 location);
    private:
		int seed = 0;
		void deleteFarChunks(int xPos, int zPos);
};
