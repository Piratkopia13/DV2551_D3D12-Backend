#include "OpenGL/OpenGLRenderer.h"
#include "DX12/DX12Renderer.h"
#include "Renderer.h"


Renderer* Renderer::makeRenderer(BACKEND option)
{
	if (option == BACKEND::GL45)
		return new OpenGLRenderer();
	if (option == BACKEND::DX12)
		return new DX12Renderer();
}

