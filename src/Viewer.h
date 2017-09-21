#pragma once

#include <nsessentials/gui/AbstractViewer.h>

#include "Content.h"

//The viewer displays some generic content. It contains functionality for camera control and interaction.
class Viewer : public nse::gui::AbstractViewer
{
public:
	Viewer(Content* content);
	~Viewer();

	void drawContents();

private:

	Content* content;	
};