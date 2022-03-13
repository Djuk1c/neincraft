#include "Chunk.h"

Chunk::Chunk(int x, int z, int seed)
{
	facesCount = 0;
	Simplex::seed(seed);	// Usporava?
    xPos = x;
    zPos = z;
    // Fill the chunk with empty blocks
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
				if (y == 0)
					chunk[x][y][z] = block_water;
				else
					chunk[x][y][z] = block_air;
            }
        }
    }

    // Fill it with simplex noise
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
				float smoothness = 64.0f + 16.0f;
                float val = Simplex::noise(glm::vec2(((float)x + (CHUNK_SIZE * xPos)) / smoothness, ((float)z + (CHUNK_SIZE * zPos)) / smoothness));
				for (int i = 1; i < 20; i++)
				{
					val += (Simplex::noise(glm::vec2(((float)x + 64*i + (CHUNK_SIZE * xPos)) / smoothness/2, ((float)z + 64*i + (CHUNK_SIZE * zPos)) / smoothness/2))) / 2.0f;
				}
				val -= 1;
				val = val * 2 - 1;
                val *= 4;     // Add more height variation
                if (val + y < -4)    // Height chunka
                    chunk[x][y][z] = block_dirt;
            }
        }
    }
}

Chunk::~Chunk()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBOMesh);
	std::cout << "Deleted chunk X:" << xPos << " Z:" << zPos << std::endl;
}

void Chunk::generateMesh()
{
	class cubeData
	{
		public:
			bool backface, bottomface, leftface, frontface, topface, rightface;
			cubeData()
			{
				backface = true;
				bottomface = true;
				leftface = true;
				frontface = true;
				topface = true;
				rightface = true;
			}
	};
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                if (chunk[x][y][z] != block_air)
                {
                    cubeData newCube;
                    if (x > 0) newCube.leftface = chunk[x - 1][y][z] == 0;
                    if (x < CHUNK_SIZE - 1) newCube.rightface = chunk[x + 1][y][z] == 0;
                    if (y > 0) newCube.bottomface = chunk[x][y - 1][z] == 0;
                    if (y < CHUNK_SIZE - 1) newCube.topface = chunk[x][y + 1][z] == 0;
                    if (z > 0) newCube.frontface = chunk[x][y][z - 1] == 0;
                    if (z < CHUNK_SIZE - 1) newCube.backface = chunk[x][y][z + 1] == 0;

                    addCube(newCube.backface, newCube.bottomface, newCube.leftface, newCube.frontface, newCube.topface, newCube.rightface, x + (xPos * CHUNK_SIZE), y, z + (zPos * CHUNK_SIZE), chunk[x][y][z], chunkData);
                }
            }
        }
    }
}

void Chunk::generateVBO()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// Create Vertex Buffer Objects
	glGenBuffers(1, &VBOMesh);
	glBindBuffer(GL_ARRAY_BUFFER, VBOMesh);
	glBufferData(GL_ARRAY_BUFFER, chunkData.size() * sizeof(float), &chunkData.front(), GL_STATIC_DRAW);
	// Link
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));   // Pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));   // TexCoord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));   // Light
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));   // Index
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));   // TexId
	glEnableVertexAttribArray(4);
	// Unbinds
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Chunk::addCube(bool backface, bool bottomface, bool leftface, bool frontface, bool topface, bool rightface, int x, int y, int z, int texId, std::vector<float> &chunkData)
{
    // 3 pos 2 texcoord 1 light 1 index
    if (backface)
	{
		facesCount++;
		chunkData.insert(chunkData.end(),
		{
			-0.5f + x, -0.5f + y,  0.5f + z,  0.0f, 0.0f, 0.6f, 1.0f, (float)texId,
			 0.5f + x, -0.5f + y,  0.5f + z,  1.0f, 0.0f, 0.6f, 1.0f, (float)texId,
			 0.5f + x,  0.5f + y,  0.5f + z,  1.0f, 1.0f, 0.6f, 1.0f, (float)texId,
			 0.5f + x,  0.5f + y,  0.5f + z,  1.0f, 1.0f, 0.6f, 1.0f, (float)texId,
			-0.5f + x,  0.5f + y,  0.5f + z,  0.0f, 1.0f, 0.6f, 1.0f, (float)texId,
			-0.5f + x, -0.5f + y,  0.5f + z,  0.0f, 0.0f, 0.6f, 1.0f, (float)texId,
		});
	}
    if (bottomface)
	{
		facesCount++;
		//y -= 1;
		chunkData.insert(chunkData.end(),
		{
			-0.5f + x, -0.5f + y, -0.5f + z,  0.0f, 1.0f, 0.1f, 2.0f, (float)texId,
			 0.5f + x, -0.5f + y, -0.5f + z,  1.0f, 1.0f, 0.1f, 2.0f, (float)texId,
			 0.5f + x, -0.5f + y,  0.5f + z,  1.0f, 0.0f, 0.1f, 2.0f, (float)texId,
			 0.5f + x, -0.5f + y,  0.5f + z,  1.0f, 0.0f, 0.1f, 2.0f, (float)texId,
			-0.5f + x, -0.5f + y,  0.5f + z,  0.0f, 0.0f, 0.1f, 2.0f, (float)texId,
			-0.5f + x, -0.5f + y, -0.5f + z,  0.0f, 1.0f, 0.1f, 2.0f, (float)texId,
		});
	}
    if (leftface)
	{
		facesCount++;
		//z -= 1;
		chunkData.insert(chunkData.end(),
		{
			-0.5f + x,  0.5f + y,  0.5f + z,  1.0f, 1.0f, 1.0f, 3.0f, (float)texId,
			-0.5f + x,  0.5f + y, -0.5f + z,  0.0f, 1.0f, 1.0f, 3.0f, (float)texId,
			-0.5f + x, -0.5f + y, -0.5f + z,  0.0f, 0.0f, 1.0f, 3.0f, (float)texId,
			-0.5f + x, -0.5f + y, -0.5f + z,  0.0f, 0.0f, 1.0f, 3.0f, (float)texId,
			-0.5f + x, -0.5f + y,  0.5f + z,  1.0f, 0.0f, 1.0f, 3.0f, (float)texId,
			-0.5f + x,  0.5f + y,  0.5f + z,  1.0f, 1.0f, 1.0f, 3.0f, (float)texId,
		});
	}
    if (frontface)
	{
		facesCount++;
		//x += 1;
		chunkData.insert(chunkData.end(),
		{
			-0.5f + x , -0.5f + y, -0.5f + z,  0.0f, 0.0f, 0.7f, 4.0f, (float)texId,
			 0.5f + x , -0.5f + y, -0.5f + z,  1.0f, 0.0f, 0.7f, 4.0f, (float)texId,
			 0.5f + x ,  0.5f + y, -0.5f + z,  1.0f, 1.0f, 0.7f, 4.0f, (float)texId,
			 0.5f + x ,  0.5f + y, -0.5f + z,  1.0f, 1.0f, 0.7f, 4.0f, (float)texId,
			-0.5f + x ,  0.5f + y, -0.5f + z,  0.0f, 1.0f, 0.7f, 4.0f, (float)texId,
			-0.5f + x , -0.5f + y, -0.5f + z,  0.0f, 0.0f, 0.7f, 4.0f, (float)texId,
		});
	}
    if (topface)
	{
		facesCount++;
		//y += 1;
		chunkData.insert(chunkData.end(),
		{
			-0.5f + x,  0.5f + y, -0.5f + z,  0.0f, 1.0f, 0.9f, 5.0f, (float)texId,
			 0.5f + x,  0.5f + y, -0.5f + z,  1.0f, 1.0f, 0.9f, 5.0f, (float)texId,
			 0.5f + x,  0.5f + y,  0.5f + z,  1.0f, 0.0f, 0.9f, 5.0f, (float)texId,
			 0.5f + x,  0.5f + y,  0.5f + z,  1.0f, 0.0f, 0.9f, 5.0f, (float)texId,
			-0.5f + x,  0.5f + y,  0.5f + z,  0.0f, 0.0f, 0.9f, 5.0f, (float)texId,
			-0.5f + x,  0.5f + y, -0.5f + z,  0.0f, 1.0f, 0.9f, 5.0f, (float)texId,

		});
	}
    if (rightface)
	{
		facesCount++;
		//z += 1;
		chunkData.insert(chunkData.end(),
		{
			 0.5f + x,  0.5f + y,  0.5f + z,  1.0f, 1.0f, 0.5f, 6.0f, (float)texId,
			 0.5f + x,  0.5f + y, -0.5f + z,  0.0f, 1.0f, 0.5f, 6.0f, (float)texId,
			 0.5f + x, -0.5f + y, -0.5f + z,  0.0f, 0.0f, 0.5f, 6.0f, (float)texId,
			 0.5f + x, -0.5f + y, -0.5f + z,  0.0f, 0.0f, 0.5f, 6.0f, (float)texId,
			 0.5f + x, -0.5f + y,  0.5f + z,  1.0f, 0.0f, 0.5f, 6.0f, (float)texId,
			 0.5f + x,  0.5f + y,  0.5f + z,  1.0f, 1.0f, 0.5f, 6.0f, (float)texId,
		});
	}
}
