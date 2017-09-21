#pragma once

#include "Content.h"
#include <nanogui/opengl.h>

class TriangleContent : public Content
{
public:

	~TriangleContent();

	void Draw(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, const Eigen::Matrix4f& viewProjection);

	void CreateGeometry();

	void SetupGUI(NVGcontext* ctx, nanogui::Widget* parent) { }

private:
	GLuint vao;
	GLuint vboPositions, vboColors;
};