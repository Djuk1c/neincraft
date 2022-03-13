#include "World.h"

void World::generateWorld(glm::vec3 cameraPos)
{
	srand(time(NULL));
	seed = rand()%1337420;

    // Get current player position in chunk coords
    int xPos = (int)ceil(cameraPos.x / 16);
    int zPos = (int)ceil(cameraPos.z / 16);

    // Generate vector with wanted chunks
    for (int x = xPos - CHUNKS_NUM; x < xPos + CHUNKS_NUM; x++)
    {
        for (int z = zPos - CHUNKS_NUM; z < zPos + CHUNKS_NUM; z++)
        {
            Chunk chunk(x, z, seed);
            chunk.generateMesh();
			chunk.generateVBO();
			worldChunks.insert(std::make_pair(std::make_pair(x, z), chunk));
        }
    }
}

void World::addNearbyChunks(glm::vec3 location)
{
    // Get current player position in chunk coords
    int xPos = (int)ceil(location.x / 16);
    int zPos = (int)ceil(location.z / 16);

	deleteFarChunks(xPos, zPos);

	for (int x = -CHUNKS_NUM; x < CHUNKS_NUM; x++)
	{
		for (int z = -CHUNKS_NUM; z < CHUNKS_NUM; z++)
		{
			addChunkAtLocation(xPos+x, zPos+z);
		}
	}
}

bool World::addChunkAtLocation(int xPos, int zPos)
{
	if (worldChunks.find(std::make_pair(xPos, zPos)) != worldChunks.end())
			return false;

	Chunk chunk(xPos, zPos, seed);
	chunk.generateMesh();
	chunk.generateVBO();
	worldChunks.insert(std::make_pair(std::make_pair(xPos, zPos), chunk));
	return true;
}

void World::deleteFarChunks(int xPos, int zPos)
{
	std::map<std::pair<int, int>, Chunk>::iterator it = worldChunks.begin();
	while (it != worldChunks.end())
	{
		int x = std::abs(xPos);
		int z = std::abs(zPos);
		int chunkX = std::abs(it->first.first);
		int chunkZ = std::abs(it->first.second);

		if (std::abs(x - chunkX) > CHUNKS_NUM+1 || std::abs(z - chunkZ) > CHUNKS_NUM+1)
		{
			glDeleteVertexArrays(1, &it->second.VAO);
			glDeleteBuffers(1, &it->second.VBOMesh);
			worldChunks.erase(it++);
		}
		else
		{
			++it;
		}
	}
}