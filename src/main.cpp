#include <iostream>
#include <string>

#include <nsessentials/util/GLDebug.h>

#include "Viewer.h"
#include "Content.h"
#include "TriangleContent.h"
#include "HeightFieldContent.h"
#include "SimpleOBJContent.h"
#include "IlluminatedMeshContent.h"
#include "TexturedMeshContent.h"

const std::string dataDirectory = "../data/";

int main()
{
	std::cout << "Please choose the content to show: " << std::endl;
	std::cout << "\t1:\tSimple triangle" << std::endl;
	std::cout << "\t2:\tSimple heightfield with indexed rendering" << std::endl;
	std::cout << "\t3:\tSimple OBJ geometry" << std::endl;
	std::cout << "\t4:\tIlluminated Mesh" << std::endl;
	std::cout << "\t5:\tTextured Mesh" << std::endl;
	
	int choice;
	std::cin >> choice;

	Content* content = nullptr;
	switch (choice)
	{
	case 1: content = new TriangleContent(); break;
	case 2: content = new HeightFieldContent(); break;
	case 3: content = new SimpleOBJContent(dataDirectory); break;
	case 4: content = new IlluminatedMeshContent(dataDirectory); break;
	case 5: content = new TexturedMeshContent(dataDirectory); break;
	}

	if (content == nullptr)
	{
		std::cout << "No valid content chosen." << std::endl;
		return 0;
	}

	std::cout << "Starting viewer ..." << std::endl;
	nanogui::init();

	{
		nanogui::ref<Viewer> viewer = new Viewer(content);
		viewer->setVisible(true);

		nse::util::GLDebug::SetupDebugCallback();
		nse::util::GLDebug::IgnoreGLError(131185); //buffer usage info

		try
		{
			nanogui::mainloop(16);
		}
		catch (std::runtime_error& e)
		{
			std::cerr << e.what() << std::endl;
		}

	}

	nanogui::shutdown();

	return 0;
}