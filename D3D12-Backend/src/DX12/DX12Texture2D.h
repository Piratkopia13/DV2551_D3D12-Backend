#pragma once
#include "../Texture2D.h"
#include "DX12.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


class DX12Texture2D : public Texture2D
{
public:
	DX12Texture2D();
	~DX12Texture2D();

	int loadFromFile(std::string filename);
	void bind(unsigned int slot);
	Sampler2D* sampler = nullptr;

	//DX HANDLE
	//something?
};

