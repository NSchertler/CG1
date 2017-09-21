#pragma once

#include "Content.h"
#include <nsessentials/gui/GLBuffer.h>
#include <nsessentials/gui/GLVertexArray.h>

#include <string>

class SimpleOBJContent : public Content
{
public:

	SimpleOBJContent(const std::string& dataDirectory);

	void Draw(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, const Eigen::Matrix4f& viewProjection);

	void CreateGeometry();

	void SetupGUI(NVGcontext* ctx, nanogui::Widget* parent) { }

private:
	nse::gui::GLVertexArray vao;
	nse::gui::GLBuffer vertexBuffer;
	nse::gui::GLBuffer indexBuffer;
	nse::gui::GLBuffer instanceDataBuffer;

	unsigned int indexCount;

	std::string dataDirectory;
};