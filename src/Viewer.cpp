#include "Viewer.h"
#include "ShaderPool.h"

#include <iostream>

#define USE_3D_CAMERA

Viewer::Viewer(Content* content)
	: AbstractViewer("CG1 Examples"), content(content)
{
	std::cout << "Compiling shaders ..." << std::endl;
	ShaderPool::Instance()->CompileAll();	

	content->SetupGUI(nvgContext(), this);	
	performLayout(nvgContext());

	content->CreateGeometry();
	camera().FocusOnBBox(content->BoundingBox);
}

Viewer::~Viewer()
{
	delete content;
}

void Viewer::drawContents()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	//Calculate camera matrices
	Eigen::Matrix4f view, proj;
#ifndef USE_3D_CAMERA
	view = Eigen::Matrix4f::Identity();
	proj = Eigen::Matrix4f::Identity();
#else
	//Or if we want a real camera:
	camera().ComputeCameraMatrices(view, proj);	
#endif
	Eigen::Matrix4f viewProj = proj * view;
	
	content->Draw(view, proj, viewProj);
}