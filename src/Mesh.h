#pragma once

#include <nsessentials/gui/GLBuffer.h>
#include <nsessentials/gui/GLVertexArray.h>
#include <nsessentials/math/BoundingBox.h>

//Wrapper class for a vertex buffer and vertex array object of a triangle mesh
class Mesh
{
public:
	struct Vertex
	{
		Eigen::Vector3f position;
		Eigen::Vector3f normal;
		Eigen::Vector2f texCoord;
	};

	enum DrawMode
	{
		Triangles,
		Points,
		Patches
	};

	Mesh();

	void LoadModel(const std::string& file);
	void Draw(DrawMode drawMode = Triangles);

	const nse::math::BoundingBox<float, 3>& BoundingBox() const { return bbox; }

private:
	nse::gui::GLVertexArray vao;
	nse::gui::GLBuffer vertexBuffer;
	nse::gui::GLBuffer indexBuffer;

	unsigned int indexCount;

	nse::math::BoundingBox<float, 3> bbox;

	//This function returns the index of the provided (v, n, t) triplet within vertices.
	//If this triplet does not yet exist, a new vertex is generated from this triplet.
	unsigned int GetVertexIndex(unsigned int v, unsigned int n, unsigned int t,
		std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int>& tripletToVertexIndex,
		const std::vector<Eigen::Vector3f>& positions,
		const std::vector<Eigen::Vector3f>& normals,
		const std::vector<Eigen::Vector2f>& texCoords,
		std::vector<Vertex>& vertices);
};