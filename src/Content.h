#pragma once

#include <Eigen/Dense>
#include <nanogui/common.h>

#include <nsessentials/math/BoundingBox.h>

//Abstract base class for rendering content.
class Content
{
public:
	virtual void Draw(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, const Eigen::Matrix4f& viewProjection) = 0;

	virtual void CreateGeometry() = 0;

	virtual void SetupGUI(NVGcontext* ctx, nanogui::Widget* parent) = 0;

	nse::math::BoundingBox<float, 3> BoundingBox;
};