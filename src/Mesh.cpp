#include "Mesh.h"

#include <tinyobjloader/tiny_obj_loader.h>
#include <iostream>

Mesh::Mesh()
	: vertexBuffer(nse::gui::VertexBuffer), indexBuffer(nse::gui::IndexBuffer)
{ }

//This function returns the index of the provided (v, n, t) triplet within vertices.
//If this triplet does not yet exist, a new vertex is generated from this triplet.
unsigned int Mesh::GetVertexIndex(unsigned int v, unsigned int n, unsigned int t,
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

void Mesh::LoadModel(const std::string& file)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::cout << "Loading mesh.." << std::endl;

	//Load OBJ file
	std::string err;
	bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str());

	if (!err.empty())
		std::cout << err;
	if (!success)
		throw;

	bbox.reset();

	//Write the attributes into a container that is nicer to handle.
	//Actually, this is an unnecessary step and reduces loading performance. However,
	//it makes the rest of the code more readable.
	std::vector<Eigen::Vector3f> positions(attrib.vertices.size() / 3);
	std::vector<Eigen::Vector3f> normals(attrib.normals.size() / 3);
	std::vector<Eigen::Vector2f> texCoords(attrib.texcoords.size() / 2);

	for (int i = 0; i < attrib.vertices.size() / 3; ++i)
	{
		positions[i] = Eigen::Vector3f(attrib.vertices[3 * i + 0], attrib.vertices[3 * i + 1], attrib.vertices[3 * i + 2]);
		bbox.expand(positions[i]);
	}

	for (int i = 0; i < attrib.normals.size() / 3; ++i)
		normals[i] = Eigen::Vector3f(attrib.normals[3 * i + 0], attrib.normals[3 * i + 1], attrib.normals[3 * i + 2]);

	for (int i = 0; i < attrib.texcoords.size() / 2; ++i)
		texCoords[i] = Eigen::Vector2f(attrib.texcoords[2 * i + 0], 1 - attrib.texcoords[2 * i + 1]);

	if (normals.size() == 0)
	{
		//calculate normals
		normals.resize(positions.size(), Eigen::Vector3f::Zero());
		for (int i = 0; i < shapes[0].mesh.indices.size(); ++i)
			shapes[0].mesh.indices[i].normal_index = shapes[0].mesh.indices[i].vertex_index;

		for (int i = 0; i < shapes[0].mesh.indices.size(); i += 3)
		{
			int idx[] = { shapes[0].mesh.indices[i].vertex_index, shapes[0].mesh.indices[i + 1].vertex_index, shapes[0].mesh.indices[i + 2].vertex_index };
			Eigen::Vector3f faceNormal = (positions[idx[1]] - positions[idx[0]]).cross(positions[idx[2]] - positions[idx[0]]).normalized();
			for (int j = 0; j < 3; ++j)
			{
				Eigen::Vector3f diffPrev = positions[idx[(j + 2) % 3]] - positions[idx[j]];
				Eigen::Vector3f diffNext = positions[idx[(j + 1) % 3]] - positions[idx[j]];
				float dotProduct = diffNext.dot(diffPrev) / (diffNext.norm() * diffPrev.norm());
				if (dotProduct > 1)
					dotProduct = 1;
				if (dotProduct < -1)
					dotProduct = -1;
				normals[idx[j]] += faceNormal * acos(dotProduct); //average face normals by angle
			}
		}

		for (int i = 0; i < normals.size(); ++i)
			normals[i].normalize();
	}

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

	indexBuffer.uploadData((uint32_t)indices.size() * sizeof(unsigned int), indices.data());

	vao.unbind();
}

void Mesh::Draw(DrawMode drawMode)
{
	if (indexCount == 0)
		return;

	vao.bind();

	GLenum primitiveType;
	switch (drawMode)
	{
	case Triangles: primitiveType = GL_TRIANGLES; break;
	case Points: primitiveType = GL_POINTS; break;
	case Patches: primitiveType = GL_PATCHES; glPatchParameteri(GL_PATCH_VERTICES, 3); break;
	}

	glDrawElements(primitiveType, indexCount, GL_UNSIGNED_INT, 0);
	vao.unbind();
}