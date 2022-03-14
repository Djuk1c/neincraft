#pragma once
#include <iostream>
#include <vector>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "FastNoiseLite.h"

#define CHUNK_SIZE 		16
#define CHUNK_HEIGHT	64

using namespace std;

class Chunk
{
    public:
        std::vector<float> chunkData;
		unsigned int VAO;
		unsigned int VBOMesh;
		unsigned int facesCount;
        Chunk(int x, int z, FastNoiseLite &noise);
		~Chunk();
        void generateMesh();
		void generateVBO();
        enum blocks
        {
            block_air, block_dirt, block_water
        };
        int chunk[CHUNK_SIZE][128][CHUNK_SIZE];
        int xPos, zPos;

    private:
        void addCube(bool backface, bool bottomface, bool leftface, bool frontface, bool topface, bool rightface, int x, int y, int z, int texId, std::vector<float> &chunkData);
};
