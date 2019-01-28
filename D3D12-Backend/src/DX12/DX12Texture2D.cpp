#include "DX12Texture2D.h"



DX12Texture2D::DX12Texture2D(DX12Renderer* _renderer) {
	renderer = _renderer;
}


DX12Texture2D::~DX12Texture2D() {

}


// return 0 if image was loaded and texture created.
// else return -1
int DX12Texture2D::loadFromFile(std::string filename) {
	
	//DXGI_FORMAT_R8G8B8A8_UNORM

	int w, h, bpp;
	unsigned char* rgb = stbi_load(filename.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	if (rgb == nullptr) {
		return -1;
	}

	D3D12_RESOURCE_DESC textureDesc;
	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	//HRESULT hr = renderer->getDevice()->CreateCommittedResource(
	//	&DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), //&, a default heap               -- ?
	//	D3D12_HEAP_FLAG_NONE, // no flags
	//	&textureDesc, // the description of our texture
	//	D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
	//	nullptr, // used for render targets and depth/stencil buffers
	//	IID_PPV_ARGS(&textureBuffer));
	//if (FAILED(hr))
	//	return -1;
	
	textureBuffer->SetName(L"Texture Buffer Resource Heap");
	



	return 0;
}

void DX12Texture2D::bind(unsigned int slot) {

}
