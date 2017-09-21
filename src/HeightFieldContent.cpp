#include "HeightFieldContent.h"

#include "ShaderPool.h"

void HeightFieldContent::CreateGeometry()
{
	//We use an indexed representation
	std::vector<Eigen::Vector4f> gridPositions;
	std::vector<Eigen::Vector4f> gridColors;
	std::vector<unsigned int> indices;

	const int resolution = 20;
	auto heightfieldFunction = [](float x, float y) { return 0.6f * sin(2 * x) + 0.3f * sin(3 * y); };

	//Generate the vertices
	for (int v = 0; v < resolution; ++v)
		for (int u = 0; u < resolution; ++u)
		{
			float x = 2 * ((2.0f * u) / resolution - 1.0f);
			float y = 2 * ((2.0f * v) / resolution - 1.0f);
			float z = heightfieldFunction(x, y);

			float grayScale = 0.5f * (z + 1);

			gridPositions.push_back(Eigen::Vector4f(x, y, z, 1));
			gridColors.push_back(Eigen::Vector4f(grayScale, grayScale, grayScale, 1));
		}

	//Generate the faces
	for (int v = 0; v < resolution - 1; ++v)
		for (int u = 0; u < resolution - 1; ++u)
		{
			//Add the according faces
			//       u  u+1
			//   --- +---+ --- v+1
			//       | / |
			//   --- +---+ --- v
			//       |   |
			//start by calculating the indices of the corner
			unsigned int bottomLeft = u + v       * resolution;
			unsigned int bottomRight = u + 1 + v       * resolution;
			unsigned int topLeft = u + (v + 1) * resolution;
			unsigned int topRight = u + 1 + (v + 1) * resolution;

			//left triangle
			indices.push_back(bottomLeft);
			indices.push_back(topRight);
			indices.push_back(topLeft);

			//right triangle
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
			indices.push_back(topRight);
		}
	indexCount = (unsigned int)indices.size();

	//and upload everything to the GPU
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vboPositions);
	glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	glBufferData(GL_ARRAY_BUFFER, gridPositions.size() * sizeof(Eigen::Vector4f), gridPositions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector4f), 0);

	glGenBuffers(1, &vboColors);
	glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	glBufferData(GL_ARRAY_BUFFER, gridColors.size() * sizeof(Eigen::Vector4f), gridColors.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector4f), 0);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	BoundingBox.min = Eigen::Vector3f(-2.0f, -2.0f, -1.0f);
	BoundingBox.max = Eigen::Vector3f(2.0f, 2.0f, 1.0f);
}

void HeightFieldContent::Draw(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, const Eigen::Matrix4f& viewProjection)
{
	//bind the current shader
	ShaderPool::Instance()->SimpleShader.bind();

	//set uniform variables
	ShaderPool::Instance()->SimpleShader.setUniform("modelViewProjection", viewProjection);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

HeightFieldContent::~HeightFieldContent()
{
	glDeleteBuffers(1, &vboPositions);
	glDeleteBuffers(1, &vboColors);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vao);
}