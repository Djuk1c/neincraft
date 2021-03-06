#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "FastNoiseLite.h"

#define CHUNK_SIZE 		16
#define CHUNK_HEIGHT	64

using namespace std;

class Chunk
{
    private:
		struct cubeData
		{
			bool backface, bottomface, leftface, frontface, topface, rightface;
			cubeData()
			{
				backface = false;
				bottomface = false;
				leftface = false;
				frontface = false;
				topface = false;
				rightface = false;
			}
			void fillAll()
			{
				backface = true;
				bottomface = true;
				leftface = true;
				frontface = true;
				topface = true;
				rightface = true;
			}
			void clearAll()
			{
				backface = false;
				bottomface = false;
				leftface = false;
				frontface = false;
				topface = false;
				rightface = false;
			}
		};
    public:
        Chunk(int x, int z, FastNoiseLite &noise, std::map<std::pair<int, int>, Chunk*> *worldChunks);
		~Chunk();
		void generateCubeData();
		void fillConnectingCubes();
        void generateMesh();
		void generateVBO();
		void updateVBO();
		void addTree();

        int chunk[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];		// Perlin
		cubeData faces[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];	// Faces

        std::vector<float> chunkData;							// Vertex data
        int xPos, zPos;
		unsigned int VAO;
		unsigned int VBO;
		unsigned int facesCount;
		std::map<std::pair<int, int>, Chunk*> *worldChunks;
        enum blocks
        {
            block_air, block_dirt, block_dirtNoGrass, block_water, block_tree, block_leaves
        };

    private:
        void addCube(bool backface, bool bottomface, bool leftface, bool frontface, bool topface, bool rightface, int x, int y, int z, int texId, std::vector<float> &chunkData);
		FastNoiseLite noise;
		int generatePerlin(int x, int y, int z, int xPos, int zPos);
};

