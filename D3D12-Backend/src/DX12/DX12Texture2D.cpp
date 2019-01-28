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
	unsigned char* rgba = stbi_load(filename.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	if (rgba == nullptr) {
		return -1;
	}

	D3D12_RESOURCE_DESC textureDesc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		w,h,
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UINT,
		{},											//WAT IS DIS
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_NONE
	};

	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	HRESULT hr = renderer->getDevice()->CreateCommittedResource(
		0,///&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&textureDesc, // the description of our texture
		D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
		nullptr, // used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&textureBuffer));
	if (FAILED(hr)) 
		return false; 
	textureBuffer->SetName(L"Texture Buffer Resource Heap");

	UINT64 textureUploadBufferSize;
	// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
	// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
	// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
	//textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
	renderer->getDevice()->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	// now we create an upload heap to upload our texture to the GPU
	hr = renderer->getDevice()->CreateCommittedResource(
		0,///&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		0,//&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS(&textureBufferUploadHeap));
	if (FAILED(hr)) 
		return false; 

	textureBufferUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &rgba[0]; // pointer to our image data
	textureData.RowPitch = w; /// size of all our triangle vertex data
	textureData.SlicePitch = w * textureDesc.Height; /// also the size of our triangle vertex data

	// Now we copy the upload buffer contents to the default heap
	
	
	///UpdateSubresources(commandList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);
	
	// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	renderer->getCmdList()->ResourceBarrier(1, 0);/// &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// create the descriptor heap that will store our srv
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = renderer->getDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap));
	if (FAILED(hr))
		return -1;

	// now we create a shader resource view (descriptor that points to the texture and describes it)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	renderer->getDevice()->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Now we execute the command list to upload the initial assets (triangle data)
	renderer->getCmdList()->Close();
	ID3D12CommandList* ppCommandLists[] = { renderer->getCmdList() };
	renderer->getCmdQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	/// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	//fenceValue[frameIndex]++;
	//hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	//if (FAILED(hr))
	//{
	//	Running = false;
	//	return false;
	//}

	// we are done with image data now that we've uploaded it to the gpu, so free it up
	delete rgba;


	return 0;
}

void DX12Texture2D::bind(unsigned int slot) {

}
