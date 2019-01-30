#pragma once
#include "../Texture2D.h"
#include "DX12.h"

#include "DX12Renderer.h"
#include "DX12Sampler2D.h"


class DX12Texture2D : public Texture2D
{
public:
	DX12Texture2D(DX12Renderer * _renderer);
	~DX12Texture2D();

	int loadFromFile(std::string filename);
	void bind(unsigned int slot);

private:
	DX12Renderer * renderer;


	ID3D12Resource* textureBuffer;
	ID3D12DescriptorHeap* mainDescriptorHeap;
	ID3D12Resource* textureBufferUploadHeap;
};

