#include "TexturedMeshContent.h"

#include "ShaderPool.h"

#include <stb_image.h>
#include <memory>

#include <iostream>

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>


TexturedMeshContent::TexturedMeshContent(const std::string & dataDirectory)
	: dataDirectory(dataDirectory)
{
}

void DrawMesh(Mesh& mesh, nse::gui::GLShader& shader, GLuint albedoMap, GLuint normalMap, GLuint displacementMap, bool parallaxMapping)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, albedoMap);
	shader.setUniform("albedoMap", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	shader.setUniform("normalMap", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, displacementMap);
	shader.setUniform("displacementMap", 2);

	if (parallaxMapping)
	{
		mesh.Draw();
	}
	else
	{
		mesh.Draw(Mesh::Patches);
	}
}

void TexturedMeshContent::Draw(const Eigen::Matrix4f & view, const Eigen::Matrix4f & projection, const Eigen::Matrix4f & viewProjection)
{
	nse::gui::GLShader& shader = (rdParallaxMapping->pushed() ? ShaderPool::Instance()->TexturedMeshShader : ShaderPool::Instance()->TexturedMeshTessellationShader);
	shader.bind();	
	shader.setUniform("modelView", view);
	Eigen::Vector3f lightPosInViewSpace = Eigen::Vector3f::Zero();
	shader.setUniform("lightPosInViewSpace", lightPosInViewSpace);

	shader.setUniform("useAlbedoMap", chkAlbedo->checked() ? 1 : 0);
	shader.setUniform("useNormalMap", chkNormal->checked() ? 1 : 0);
	shader.setUniform("useDisplacementMap", chkDisplacement->checked() ? 1 : 0);

	shader.setUniform("displacementScale", (float)sldScale->value());

	if (rdParallaxMapping->pushed())
	{
		shader.setUniform("modelViewProjection", viewProjection);
	}
	else
	{
		shader.setUniform("projection", projection);
		shader.setUniform("tessellationLevel", sldTessellationLevel->value());
	}

	if(chkWireframe->checked())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	DrawMesh(body, shader, bodyAlbedo, bodyNormal, bodyDisplacement, rdParallaxMapping->pushed());
	DrawMesh(eyes, shader, eyesAlbedo, neutralNormal, neutralDisplacement, rdParallaxMapping->pushed());
	DrawMesh(handClaws, shader, clawsAlbedo, neutralNormal, neutralDisplacement, rdParallaxMapping->pushed());
	DrawMesh(toeClaws, shader, clawsAlbedo, neutralNormal, neutralDisplacement, rdParallaxMapping->pushed());
	DrawMesh(teeth, shader, teethAlbedo, neutralNormal, neutralDisplacement, rdParallaxMapping->pushed());
	DrawMesh(tongue, shader, tongueAlbedo, neutralNormal, neutralDisplacement, rdParallaxMapping->pushed());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

GLuint LoadTexture(const std::string& path, bool hdrHeightmap = false)
{
	std::cout << "Loading texture ..." << std::endl;
	
	//width, height, and number of channels of the loaded texture
	int w, h, n;
	
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	if (hdrHeightmap)
	{
		//The image will be loaded with a flipped v-axis.
		using handleType = std::unique_ptr<float[], void(*)(void*)>;
		handleType textureData(stbi_loadf(path.c_str(), &w, &h, &n, 1), stbi_image_free);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, textureData.get());
	}
	else
	{
		//The image will be loaded with a flipped v-axis.
		using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
		handleType textureData(stbi_load(path.c_str(), &w, &h, &n, 3), stbi_image_free);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.get());
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

void TexturedMeshContent::CreateGeometry()
{
	body.LoadModel(dataDirectory + "trex/OBJ T-REX/T-REX.OBJ");
	eyes.LoadModel(dataDirectory + "trex/OBJ T-REX/Eyes.OBJ");
	handClaws.LoadModel(dataDirectory + "trex/OBJ T-REX/Hand nails.OBJ");
	teeth.LoadModel(dataDirectory + "trex/OBJ T-REX/Teeth.OBJ");
	toeClaws.LoadModel(dataDirectory + "trex/OBJ T-REX/Toe nails.OBJ");
	tongue.LoadModel(dataDirectory + "trex/OBJ T-REX/Tongue.OBJ");
	BoundingBox = body.BoundingBox();

	bodyAlbedo = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/T-REX.jpg");
	bodyNormal = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/T-REX normal.jpg");	
	bodyDisplacement = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/T-REX displacement.hdr", true); //load as single-channel 32 bit texture

	eyesAlbedo = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/Eyes.jpg");
	clawsAlbedo = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/claws.jpg");
	teethAlbedo = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/teeth.jpg");
	tongueAlbedo = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/tongue.jpg");

	neutralNormal = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/neutralNormal.jpg");
	neutralDisplacement = LoadTexture(dataDirectory + "trex/TEXTURES T-REX/neutralDisplacement.jpg");
}

void TexturedMeshContent::SetupGUI(NVGcontext * ctx, nanogui::Widget * parent)
{
	auto mainWindow = new nanogui::Window(parent, "Texturing Example");
	mainWindow->setPosition(Eigen::Vector2i(15, 15));
	mainWindow->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 4, 4));

	chkWireframe = new nanogui::CheckBox(mainWindow, "Wireframe");

	chkAlbedo = new nanogui::CheckBox(mainWindow, "Use Albedo Map");
	chkNormal = new nanogui::CheckBox(mainWindow, "Use Normal Map");
	chkDisplacement = new nanogui::CheckBox(mainWindow, "Use Displacement Map");

	rdParallaxMapping = new nanogui::Button(mainWindow, "Use Parallax Occlusion Mapping");
	rdParallaxMapping->setFlags(nanogui::Button::RadioButton);
	rdParallaxMapping->setPushed(true);

	rdDisplacementMapping = new nanogui::Button(mainWindow, "Use Displacement Mapping");
	rdDisplacementMapping->setFlags(nanogui::Button::RadioButton);

	new nanogui::Label(mainWindow, "Displacement Scale:");
	sldScale = new nanogui::Slider(mainWindow);
	sldScale->setRange(std::make_pair(0.0, 1.0));
	sldScale->setValue(0.5);

	new nanogui::Label(mainWindow, "TessellationLevel:");
	sldTessellationLevel = new nanogui::Slider(mainWindow);
	sldTessellationLevel->setRange(std::make_pair(1.0, 10.0));
	sldTessellationLevel->setValue(3.0);
}

TexturedMeshContent::~TexturedMeshContent()
{
	GLuint textures[] = { bodyAlbedo, bodyNormal, bodyDisplacement,
							eyesAlbedo, clawsAlbedo, teethAlbedo, tongueAlbedo,
							neutralNormal, neutralDisplacement };
	glDeleteTextures(9, textures);
}