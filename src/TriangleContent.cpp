#include "TriangleContent.h"

#include "ShaderPool.h"

void TriangleContent::CreateGeometry()
{
	//Generate geometry data
	std::vector<Eigen::Vector4f> trianglePositions;
	trianglePositions.push_back(Eigen::Vector4f(-0.5f, -0.5f, 0.0f, 1.0f));
	trianglePositions.push_back(Eigen::Vector4f(0.5f, -0.5f, 0.0f, 1.0f));
	trianglePositions.push_back(Eigen::Vector4f(0.0f, 0.5f, 0.0f, 1.0f));

	std::vector<Eigen::Vector4f> colors;
	colors.push_back(Eigen::Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
	colors.push_back(Eigen::Vector4f(0.0f, 1.0f, 0.0f, 1.0f));
	colors.push_back(Eigen::Vector4f(0.0f, 0.0f, 1.0f, 1.0f));

	//Generate a vertex buffer
	glGenBuffers(1, &vboPositions);
	glGenBuffers(1, &vboColors);

	//Generate vertex arrays
	glGenVertexArrays(1, &vao);

	//Upload data to GPU
	glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	glBufferData(GL_ARRAY_BUFFER, trianglePositions.size() * sizeof(Eigen::Vector4f), trianglePositions.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Eigen::Vector4f), colors.data(), GL_STATIC_DRAW);

	//Set up the VAO
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector4f), 0);

	glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector4f), 0);

	glBindVertexArray(0);

	//the bounding box is used to align the camera
	BoundingBox.min = Eigen::Vector3f(-0.5f, -0.5, -0.5f);
	BoundingBox.max = Eigen::Vector3f( 0.5f,  0.5,  0.5f);
}

void TriangleContent::Draw(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, const Eigen::Matrix4f& viewProjection)
{
	//bind the shader
	ShaderPool::Instance()->SimpleShader.bind();

	//set uniform variables
	ShaderPool::Instance()->SimpleShader.setUniform("modelViewProjection", viewProjection);

	//draw the triangle
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

TriangleContent::~TriangleContent()
{
	glDeleteBuffers(1, &vboPositions);
	glDeleteBuffers(1, &vboColors);
	glDeleteVertexArrays(1, &vao);
}