#include "SimpleOBJContent.h"
#include "ShaderPool.h"

#include <tinyobjloader/tiny_obj_loader.h>

#include <iostream>
#include <Eigen/Geometry>
#include <chrono>

//#define INSTANCED

SimpleOBJContent::SimpleOBJContent(const std::string& dataDirectory)
	: dataDirectory(dataDirectory),
	vertexBuffer(nse::gui::VertexBuffer), indexBuffer(nse::gui::IndexBuffer),
	instanceDataBuffer(nse::gui::VertexBuffer)
{ }

struct Vertex
{
	Eigen::Vector3f position;
	Eigen::Vector3f normal;
	Eigen::Vector2f texCoord;
};

//This function returns the index of the provided (v, n, t) triplet within vertices.
//If this triplet does not yet exist, a new vertex is generated from this triplet.
unsigned int GetVertexIndex(unsigned int v, unsigned int n, unsigned int t,
							std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int>& tripletToVertexIndex,							
							const std::vector<Eigen::Vector3f>& positions,
							const std::vector<Eigen::Vector3f>& normals,
							const std::vector<Eigen::Vector2f>& texCoords,
							std::vector<Vertex>& vertices)
{
	auto triplet = std::make_tuple(v, n, t);
	auto it = tripletToVertexIndex.find(triplet);
	if (it == tripletToVertexIndex.end())
	{
		//this triplet does not exist yet
		Eigen::Vector3f position, normal;
		Eigen::Vector2f texCoord;
		if (v < positions.size())
			position = positions[v];
		if (n < normals.size())
			normal = normals[n];
		if (t < texCoords.size())
			texCoord = texCoords[t];

		//create a new vertex
		vertices.push_back({ position, normal, texCoord });
		//save the index for the current triplet
		tripletToVertexIndex[triplet] = (unsigned int)vertices.size() - 1;
		return (unsigned int)vertices.size() - 1;
	}
	else
		//this triplet already exists, just return the index
		return it->second;
}

void SimpleOBJContent::CreateGeometry()
{
	const std::string objPath = dataDirectory + "trex/OBJ T-REX/T-REX.OBJ";

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::cout << "Loading mesh.." << std::endl;	

	//Load OBJ file
	std::string err;
	bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objPath.c_str());

	if (!err.empty())
		std::cout << err;
	if (!success)
		throw;

	BoundingBox.reset();

	//Write the attributes into a container that is nicer to handle.
	//Actually, this is an unnecessary step and reduces loading performance. However,
	//it makes the rest of the code more readable.
	std::vector<Eigen::Vector3f> positions(attrib.vertices.size() / 3);
	std::vector<Eigen::Vector3f> normals(attrib.normals.size() / 3);
	std::vector<Eigen::Vector2f> texCoords(attrib.texcoords.size() / 2);	

	for (int i = 0; i < attrib.vertices.size() / 3; ++i)
	{
		positions[i] = Eigen::Vector3f(attrib.vertices[3 * i + 0], attrib.vertices[3 * i + 1], attrib.vertices[3 * i + 2]);
		BoundingBox.expand(positions[i]);
	}

	for (int i = 0; i < attrib.normals.size() / 3; ++i)
		normals[i] = Eigen::Vector3f(attrib.normals[3 * i + 0], attrib.normals[3 * i + 1], attrib.normals[3 * i + 2]);

	for (int i = 0; i < attrib.texcoords.size() / 2; ++i)
		texCoords[i] = Eigen::Vector2f(attrib.texcoords[2 * i + 0], 1 - attrib.texcoords[2 * i + 1]);
	
	//Generate vertices 
	std::vector<Vertex> vertices;
	std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> tripletToVertexIndex;
	std::vector<unsigned int> indices;
	indices.reserve(shapes[0].mesh.indices.size());
	for (int i = 0; i < shapes[0].mesh.indices.size(); ++i)
	{
		auto& idx = shapes[0].mesh.indices[i];
		auto vertexIndex = GetVertexIndex(idx.vertex_index, idx.normal_index, idx.texcoord_index,
			tripletToVertexIndex, positions, normals, texCoords, vertices);
		indices.push_back(vertexIndex);
	}	
	indexCount = (unsigned int)indices.size();

	//Now upload everything to the GPU
	vao.generate();
	vao.bind();

	vertexBuffer.uploadData((uint32_t)vertices.size() * sizeof(Vertex), vertices.data());
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); //positions are at the beginning
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Eigen::Vector3f)); //offset to skip first position
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(Eigen::Vector3f))); //offset to skip first position and normal

#ifdef INSTANCED
	instanceDataBuffer.bind();
	//a matrix takes up 4 slots (equals 4 4D vectors)
	for (int i = 0; i < 4; ++i)
	{
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (void*)(4 * i * sizeof(float)));
		glVertexAttribDivisor(3 + i, 1);
	}
#endif

	indexBuffer.uploadData((uint32_t)indices.size() * sizeof(unsigned int), indices.data());

	vao.unbind();

	//expand bounding box by a factor of 2 to fit the small dinos
	auto diagonal = BoundingBox.diagonal();
	BoundingBox.max += 0.5f * diagonal;
	BoundingBox.min -= 0.5f * diagonal;
}

std::chrono::time_point<std::chrono::high_resolution_clock> last;
void SimpleOBJContent::Draw(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, const Eigen::Matrix4f& viewProjection)
{
	if (indexCount == 0)
		return;

	std::vector<Eigen::Matrix4f> mvpMatrices;

	//bind the current shader
#ifdef INSTANCED	
	ShaderPool::Instance()->SimpleMeshInstancedShader.bind();
#else
	ShaderPool::Instance()->SimpleMeshShader.bind();
#endif

	vao.bind();

	//set uniform variables
#ifdef INSTANCED
	mvpMatrices.push_back(viewProjection);
#else
	ShaderPool::Instance()->SimpleMeshShader.setUniform("modelViewProjection", viewProjection);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
#endif

	//Make something like a scene graph
	const int smallDinos = 8;
	const float scale = 0.1f;
	const int millisecondsPerRound = 20000;

	//get a time value for animation
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	float rotationOffset = (ms % millisecondsPerRound) / (float)millisecondsPerRound * 2 * (float)M_PI;

	Eigen::Affine3f circleCenter(Eigen::Translation3f(0.0f, 25.0f, -40.0f) * Eigen::AngleAxisf(rotationOffset, Eigen::Vector3f::UnitY()));

	for (int i = 0; i < smallDinos; ++i)
	{
		Eigen::Affine3f model = circleCenter
			* Eigen::AngleAxisf(i * 2 * (float)M_PI / smallDinos, Eigen::Vector3f::UnitY())
			* Eigen::Translation3f(35, 0, 0)
			* Eigen::UniformScaling<float>(scale);
		Eigen::Matrix4f mvp = viewProjection * model.matrix();
#ifdef INSTANCED
		mvpMatrices.push_back(mvp);
#else
		ShaderPool::Instance()->SimpleMeshShader.setUniform("modelViewProjection", mvp);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
#endif
	}

#ifdef INSTANCED
	instanceDataBuffer.uploadData(sizeof(Eigen::Matrix4f) * mvpMatrices.size(), mvpMatrices.data());
	glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, mvpMatrices.size());
#endif

	vao.unbind();
}