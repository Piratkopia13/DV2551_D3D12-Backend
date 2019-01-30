#pragma once
#include "DX12.h"
#include "../ConstantBuffer.h"

class DX12Renderer;

class DX12ConstantBuffer : public ConstantBuffer {
public:
	DX12ConstantBuffer(std::string name, unsigned int location, DX12Renderer* renderer);
	~DX12ConstantBuffer();
	void setData(const void* data, size_t size, Material* m, unsigned int location);
	void bind(Material*);

	//ID3D12DescriptorHeap* getDescriptorHeap() const;

private:

	DX12Renderer* m_renderer;

	unsigned int m_location;
	wComPtr<ID3D12DescriptorHeap>* m_mainDescriptorHeap;
	wComPtr<ID3D12Resource>* m_constantBufferUploadHeap;
	UINT8** m_cbGPUAddress;
	
	std::string m_name;
	/*GLuint location;
	GLuint handle;
	GLuint index;
	void* buff = nullptr;
	void* lastMat;*/
};

