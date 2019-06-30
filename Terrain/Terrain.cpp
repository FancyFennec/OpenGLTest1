#include "Headers/Terrain.h"



Terrain::Terrain(int width, int height) 
{
	this->width = width;
	this->height = height;
	this->pNoise = PerlinNoise(4);
}

Terrain::~Terrain()
{
}

void Terrain::generateHeightMap(float &elevation, float &frequency, unsigned int &octaves, float &persistence)
{
	heightMap = {};

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			heightMap.push_back(
				elevation * pNoise.octavePerlin(frequency * (double)i / ((double)width), frequency * (double)j / ((double)height), 0, octaves, persistence)
			);
		}
	}
}

void Terrain::generateTerrain(float elevation, float frequency, unsigned int octaves, float persistence)
{
	vertices = {};
	std::vector<glm::vec3> points = {};

	generateHeightMap(elevation, frequency, octaves, persistence);

	for (int j = 0; j < height - 1; j++) {
		for (int i = 0; i < width - 1; i++) {

			glm::vec3 v1, v2, v3, v4;

			v1.x = (float)i;
			v1.y = heightMap[getIndex(i, j)];
			v1.z = (float)j;

			v2.x = (float)i+1;
			v2.y = heightMap[getIndex(i + 1, j)];
			v2.z = (float)j;

			v3.x = (float)i+1;
			v3.y = heightMap[getIndex(i + 1, j + 1)];
			v3.z = (float)j+1;

			v4.x = (float)i;
			v4.y = heightMap[getIndex(i, j + 1)];
			v4.z = (float)j+1;

			addTriangleToMesh(v1, v2, v3);
			addTriangleToMesh(v1, v3, v4);
		}
	}
}

void Terrain::addPointToMesh(glm::vec3 v)
{
	vertices.push_back(v.x);
	vertices.push_back(v.y);
	vertices.push_back(v.z);
}

void  Terrain::addTriangleToMesh(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
	glm::vec3 n = glm::normalize(glm::cross(v1 - v2, v3 - v1));

	addPointToMesh(v1);
	addPointToMesh(n);
	addPointToMesh(glm::vec3(1.0f));

	addPointToMesh(v2);
	addPointToMesh(n);
	addPointToMesh(glm::vec3(1.0f));

	addPointToMesh(v3);
	addPointToMesh(n);
	addPointToMesh(glm::vec3(1.0f));
}
