#pragma once

#include "Content.h"

#include <string>

#include "Mesh.h"

#include <nanogui/checkbox.h>
#include <nanogui/colorwheel.h>
#include <nanogui/button.h>

class IlluminatedMeshContent : public Content
{
public:

	IlluminatedMeshContent(const std::string& dataDirectory);

	void Draw(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, const Eigen::Matrix4f& viewProjection);

	void CreateGeometry();

	void SetupGUI(NVGcontext* ctx, nanogui::Widget* parent);

private:

	Mesh mesh;

	std::string dataDirectory;

	nanogui::CheckBox* chkLightFromCamera;
	nanogui::Slider* sldAmbientIntensity;
	nanogui::ColorWheel* colAlbedo, * colLight;
	nanogui::Slider* sldLightIntensity;
	nanogui::Button* rdBlinnPhong, *rdOrenNayar;
	nanogui::Slider* sldShininess;
	nanogui::Slider* sldSpecularity;
	nanogui::Slider* sldRoughness;
	nanogui::CheckBox* chkNormals;
};