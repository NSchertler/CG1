#include "ShaderPool.h"

#include "glsl.h"

#include <iostream>

ShaderPool* ShaderPool::_instance(nullptr);

ShaderPool::ShaderPool()
{
}

ShaderPool* ShaderPool::Instance()
{
	if (_instance == nullptr)
		_instance = new ShaderPool();
	return _instance;
}

void ShaderPool::CompileAll()
{	
	SimpleShader.init("Simple Shader", std::string((const char*)simple_vert, simple_vert_size), std::string((const char*)simple_frag, simple_frag_size));
	SimpleMeshShader.init("Simple Mesh Shader", std::string((const char*)mesh_vert, mesh_vert_size), std::string((const char*)simple_frag, simple_frag_size));
	SimpleMeshInstancedShader.init("Simple Mesh Instanced Shader", std::string((const char*)meshinstanced_vert, meshinstanced_vert_size), std::string((const char*)simple_frag, simple_frag_size));
	IlluminatedMeshShader.init("Illuminated Mesh Shader", std::string((const char*)meshillum_vert, meshillum_vert_size), std::string((const char*)blinnphong_frag, blinnphong_frag_size));
	IlluminatedMeshShaderOrenNayar.init("Illuminated Mesh Shader Oren Nayar", std::string((const char*)meshillum_vert, meshillum_vert_size), std::string((const char*)orennayar_frag, orennayar_frag_size));
	NormalShader.init("Normal Shader", std::string((const char*)normal_vert, normal_vert_size), std::string((const char*)constantcolor_frag, constantcolor_frag_size), std::string((const char*)normal_geom, normal_geom_size));
	TexturedMeshShader.init("Textured Mesh Shader", std::string((const char*)texturedmesh_vert, texturedmesh_vert_size), std::string((const char*)texturedmesh_frag, texturedmesh_frag_size));
	TexturedMeshTessellationShader.define("HAS_TESSELLATION", "1");
	TexturedMeshTessellationShader.initWithTessellation("Textured Mesh Tessellation Shader", std::string((const char*)texturedmesh_vert, texturedmesh_vert_size), std::string((const char*)texturedmesh_tcs, texturedmesh_tcs_size), std::string((const char*)texturedmesh_tes, texturedmesh_tes_size), std::string((const char*)texturedmesh_frag, texturedmesh_frag_size));
}