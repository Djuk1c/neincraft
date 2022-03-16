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
		const int CHUNKS_NUM = 8;
		std::map<std::pair<int, int>, Chunk*> worldChunks;
        void generateWorld(glm::vec3 cameraPos);
		void addNearbyChunks(glm::vec3 location);
    private:
		bool addChunkAtLocation(int xPos, int zPos);
		void deleteFarChunks(int xPos, int zPos);
		void generateCubeDatas();
		void fillConnectingCubes();
		void generateMeshes();
		FastNoiseLite noise;
};
