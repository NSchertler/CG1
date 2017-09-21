#pragma once

#include "Content.h"

#include <string>

#include "Mesh.h"

#include <nanogui/slider.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>

class TexturedMeshContent : public Content
{
public:

	TexturedMeshContent(const std::string& dataDirectory);
	~TexturedMeshContent();

	void Draw(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, const Eigen::Matrix4f& viewProjection);

	void CreateGeometry();

	void SetupGUI(NVGcontext* ctx, nanogui::Widget* parent);

private:

	//Geometry
	Mesh body, eyes, handClaws, teeth, toeClaws, tongue;

	//Textures
	GLuint bodyAlbedo, bodyNormal, bodyDisplacement;
	GLuint eyesAlbedo, clawsAlbedo, teethAlbedo, tongueAlbedo;
	GLuint neutralNormal, neutralDisplacement;

	nanogui::CheckBox* chkWireframe;
	nanogui::Slider* sldScale;
	nanogui::CheckBox *chkAlbedo, *chkNormal, *chkDisplacement;
	nanogui::Slider* sldTessellationLevel;
	nanogui::Button* rdParallaxMapping, *rdDisplacementMapping;

	std::string dataDirectory;
};