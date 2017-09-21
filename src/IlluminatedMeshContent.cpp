#include "IlluminatedMeshContent.h"

#include "ShaderPool.h"

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/slider.h>
#include <nanogui/label.h>

IlluminatedMeshContent::IlluminatedMeshContent(const std::string& dataDirectory)
	: dataDirectory(dataDirectory)
{ }

void IlluminatedMeshContent::SetupGUI(NVGcontext* ctx, nanogui::Widget* parent)
{
	auto mainWindow = new nanogui::Window(parent, "Lighting Example");
	mainWindow->setPosition(Eigen::Vector2i(15, 15));
	mainWindow->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 4, 4));	

	chkNormals = new nanogui::CheckBox(mainWindow, "Draw Normals");

	new nanogui::Label(mainWindow, "Ambient Intensity:");
	sldAmbientIntensity = new nanogui::Slider(mainWindow);
	sldAmbientIntensity->setRange(std::make_pair(0.0f, 0.1f));
	sldAmbientIntensity->setValue(0.01f);

	new nanogui::Label(mainWindow, "Albedo:");
	colAlbedo = new nanogui::ColorWheel(mainWindow, nanogui::Color(1.0f, 1.0f, 1.0f, 1.0f));

	new nanogui::Label(mainWindow, "Light:");
	colLight = new nanogui::ColorWheel(mainWindow, nanogui::Color(1.0f, 1.0f, 1.0f, 1.0f));
	new nanogui::Label(mainWindow, "Light Intensity:");
	sldLightIntensity = new nanogui::Slider(mainWindow);
	sldLightIntensity->setRange(std::make_pair(0.0f, 10000.0f));
	sldLightIntensity->setValue(5000);

	chkLightFromCamera = new nanogui::CheckBox(mainWindow, "Place light at camera");
	chkLightFromCamera->setChecked(true);

	rdBlinnPhong = new nanogui::Button(mainWindow, "Use Blinn-Phong");
	rdBlinnPhong->setFlags(nanogui::Button::RadioButton);
	rdBlinnPhong->setPushed(true);

	new nanogui::Label(mainWindow, "Shininess:");
	sldShininess = new nanogui::Slider(mainWindow);
	sldShininess->setRange(std::make_pair(1.0f, 200.0f));
	sldShininess->setValue(75);

	new nanogui::Label(mainWindow, "Specularity:");
	sldSpecularity = new nanogui::Slider(mainWindow);
	sldSpecularity->setValue(1.0);

	rdOrenNayar = new nanogui::Button(mainWindow, "Use Oren-Nayar");
	rdOrenNayar->setFlags(nanogui::Button::RadioButton);

	new nanogui::Label(mainWindow, "Roughness:");
	sldRoughness = new nanogui::Slider(mainWindow);
	sldRoughness->setRange(std::make_pair(0.0f, 2.0f));
	sldRoughness->setValue(0.5);
}

void IlluminatedMeshContent::Draw(const Eigen::Matrix4f & view, const Eigen::Matrix4f & projection, const Eigen::Matrix4f & viewProjection)
{	
	auto& shader = rdBlinnPhong->pushed() ? ShaderPool::Instance()->IlluminatedMeshShader : ShaderPool::Instance()->IlluminatedMeshShaderOrenNayar;

	shader.bind();
	shader.setUniform("modelViewProjection", viewProjection);
	shader.setUniform("modelView", view);

	Eigen::Vector3f lightPosInViewSpace3 = Eigen::Vector3f::Zero();
	if (!chkLightFromCamera->checked())	
	{
		const Eigen::Vector4f lightPosInWorldSpace = Eigen::Vector4f(25.0f, 0.0f, -10.0f, 1.0f);
		Eigen::Vector4f lightPosInViewSpace = view * lightPosInWorldSpace;
		lightPosInViewSpace3 = lightPosInViewSpace.head<3>();		
	}	
	shader.setUniform("lightPosInViewSpace", lightPosInViewSpace3);

	Eigen::Vector3f ambientLight = Eigen::Vector3f::Constant(sldAmbientIntensity->value());
	shader.setUniform("ambientLight", ambientLight);

	Eigen::Vector3f lightColor = colLight->color().head<3>() * sldLightIntensity->value();
	shader.setUniform("lightColor", lightColor);

	Eigen::Vector3f albedo = colAlbedo->color().head<3>();
	shader.setUniform("albedo", albedo);

	if (rdBlinnPhong->pushed())
	{
		shader.setUniform("shininess", sldShininess->value());
		shader.setUniform("specularity", sldSpecularity->value());
	}
	else
		shader.setUniform("roughness", sldRoughness->value());

	mesh.Draw();

	if (chkNormals->checked())
	{
		ShaderPool::Instance()->NormalShader.bind();
		ShaderPool::Instance()->NormalShader.setUniform("modelViewProjection", viewProjection);
		ShaderPool::Instance()->NormalShader.setUniform("normalLength", mesh.BoundingBox().diagonal().norm() * 0.01f);
		ShaderPool::Instance()->NormalShader.setUniform("color", Eigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f));
		mesh.Draw(Mesh::Points);
	}
}

void IlluminatedMeshContent::CreateGeometry()
{
	mesh.LoadModel(dataDirectory + "trex/OBJ T-REX/T-REX.OBJ");
	BoundingBox = mesh.BoundingBox();
}
