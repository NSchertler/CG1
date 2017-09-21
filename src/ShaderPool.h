#pragma once

#include <nsessentials/gui/GLShader.h>

//Singleton. Holds all shaders during the runtime of the application.
class ShaderPool
{
private:
	static ShaderPool* _instance;
	ShaderPool();

public:
	static ShaderPool* Instance();
	void CompileAll();

	nse::gui::GLShader SimpleShader;
	nse::gui::GLShader SimpleMeshShader;
	nse::gui::GLShader SimpleMeshInstancedShader;
	nse::gui::GLShader IlluminatedMeshShader;
	nse::gui::GLShader IlluminatedMeshShaderOrenNayar;
	nse::gui::GLShader NormalShader;
	nse::gui::GLShader TexturedMeshShader;
	nse::gui::GLShader TexturedMeshTessellationShader;
};