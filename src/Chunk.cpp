#include "Chunk.h"

int Chunk::generatePerlin(int x, int y, int z, int xPos, int zPos)
{
	if (y == 0)
		return block_water;
	float smoothness = 2.0f;
	float val = noise.GetNoise(((float)x + (CHUNK_SIZE * xPos)) / smoothness, ((float)z + (CHUNK_SIZE * zPos)) / smoothness);
	val *= 50;     // Add more height
	if (val + y < 4)
		return block_dirt;
	return block_air;
}

Chunk::Chunk(int x, int z, FastNoiseLite &noise, std::map<std::pair<int, int>, Chunk*> *worldChunks)
{
	this->worldChunks = worldChunks;
	this->noise = noise;
	facesCount = 0;
	VAO = 0;
	VBO = 0;
    xPos = x;
    zPos = z;
    // Fill the chunk with empty blocks
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
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
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
				chunk[x][y][z] = generatePerlin(x, y, z, xPos, zPos);
            }
        }
    }
}

Chunk::~Chunk()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//std::cout << "Deleted chunk X:" << xPos << " Z:" << zPos << std::endl;
}

void Chunk::generateCubeData()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
                if (chunk[x][y][z] != block_air)
                {
                    cubeData newCube;
                    if (x > 0) newCube.leftface = chunk[x - 1][y][z] == 0;
                    if (x < CHUNK_SIZE - 1) newCube.rightface = chunk[x + 1][y][z] == 0;
                    if (z > 0) newCube.frontface = chunk[x][y][z - 1] == 0;
                    if (z < CHUNK_SIZE - 1) newCube.backface = chunk[x][y][z + 1] == 0;
                    if (y > 0) newCube.bottomface = chunk[x][y - 1][z] == 0;
                    if (y < CHUNK_HEIGHT - 1) newCube.topface = chunk[x][y + 1][z] == 0;

					faces[x][y][z] = newCube;
                }
            }
        }
    }
	//addTree();
}

void Chunk::addTree()			// TODO: Temporary, rewrite
{
	int rx = rand() % 16;
	int rz = rand() % 16;
	int startY;
	int height = rand()%(7-4 + 1) + 4;
	if (rand()%3 != 1 || chunk[rx][1][rz] != block_dirt || rx-2<0 || rx+2>CHUNK_SIZE-1 || rz-2<0 || rz+2>CHUNK_SIZE-1)
		return;
	
	for (int y = 0; y < CHUNK_HEIGHT; y++)
	{
		if (chunk[rx][y][rz] == block_air)
		{
			startY = y;
			break;
		}
	}
	for (int y = startY; y < startY+height; y++)
	{
		cubeData newCube;
		newCube.fillAll();
		newCube.topface = false;
		newCube.bottomface = false;
		chunk[rx][y][rz] = block_tree;
		faces[rx][y][rz] = newCube;
	}
    for (int x = rx-2; x <= rx+2; x++)
    {
        for (int z = rz-2; z <= rz+2; z++)
        {
            for (int y = startY+height; y < startY+height+3; y++)
            {
				cubeData newCube;
				newCube.fillAll();
				chunk[x][y][z] = block_leaves;
				faces[x][y][z] = newCube;
			}
		}
	}
    for (int x = rx-1; x <= rx+1; x++)
    {
        for (int z = rz-1; z <= rz+1; z++)
        {
			cubeData newCube;
			newCube.fillAll();
			chunk[x][startY+height+3][z] = block_leaves;
			faces[x][startY+height+3][z] = newCube;
		}
	}
}

void Chunk::fillConnectingCubes()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
				if (chunk[x][y][z] == block_dirt)
				{
					if (x == CHUNK_SIZE-1 && generatePerlin(0, y, z, xPos+1, zPos) == block_air)
						faces[x][y][z].rightface = true;
					if (x == 0 && generatePerlin(CHUNK_SIZE-1, y, z, xPos-1, zPos) == block_air)
						faces[x][y][z].leftface = true;
					if (z == CHUNK_SIZE-1 && generatePerlin(x, y, 0, xPos, zPos+1) == block_air)
						faces[x][y][z].backface = true;
					if (z == 0 && generatePerlin(x, y, CHUNK_SIZE-1, xPos, zPos-1) == block_air)
						faces[x][y][z].frontface = true;
				}
			}
		}
	}
}

void Chunk::generateMesh()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
                    addCube(faces[x][y][z].backface, faces[x][y][z].bottomface, faces[x][y][z].leftface, faces[x][y][z].frontface, faces[x][y][z].topface, faces[x][y][z].rightface, x + (xPos * CHUNK_SIZE), y, z + (zPos * CHUNK_SIZE), chunk[x][y][z], chunkData);
			}
		}
	}
	generateVBO();
}

void Chunk::updateVBO()
{
	chunkData.clear();
    for (int x = 0; x < CHUNK_SIZE; x++)
        for (int z = 0; z < CHUNK_SIZE; z++)
            for (int y = 0; y < CHUNK_HEIGHT; y++)
				faces[x][y][z].clearAll(); 

	generateCubeData();
	// Fill connecting cubes
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
				if (chunk[x][y][z] == block_dirt)
				{
					if (x == CHUNK_SIZE-1 && worldChunks->at(std::make_pair(xPos+1, zPos))->chunk[0][y][z] == block_air)
						faces[x][y][z].rightface = true;
					if (x == 0 && worldChunks->at(std::make_pair(xPos-1, zPos))->chunk[CHUNK_SIZE-1][y][z] == block_air)
						faces[x][y][z].leftface = true;
					if (z == CHUNK_SIZE-1 && worldChunks->at(std::make_pair(xPos, zPos+1))->chunk[x][y][0] == block_air)
						faces[x][y][z].backface = true;
					if (z == 0 && worldChunks->at(std::make_pair(xPos, zPos-1))->chunk[x][y][CHUNK_SIZE-1] == block_air)
						faces[x][y][z].frontface = true;
				}
			}
		}
	}
	generateMesh();
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, chunkData.size() * sizeof(float), &chunkData.front(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Chunk::generateVBO()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// Create Vertex Buffer Objects
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
