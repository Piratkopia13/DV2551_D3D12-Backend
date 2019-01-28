#include "DX12Texture2D.h"



DX12Texture2D::DX12Texture2D() {

}


DX12Texture2D::~DX12Texture2D() {

}


// return 0 if image was loaded and texture created.
// else return -1
int DX12Texture2D::loadFromFile(std::string filename) {
	
	//DXGI_FORMAT_R8G8B8A8_UNORM

	int w, h, bpp;
	unsigned char* rgb = stbi_load(filename.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	if(rgb == nullptr)

	return 0;
}

void DX12Texture2D::bind(unsigned int slot) {

}
