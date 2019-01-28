#pragma once

#include "../VertexBuffer.h"
#include "DX12.h"

class DX12Renderer;

class DX12VertexBuffer : public VertexBuffer
{
private:
	//cheap ref counting
	unsigned int refs = 0;

	wComPtr<D3D12_VERTEX_BUFFER_VIEW> m_vbView;

	size_t m_size;

	DX12Renderer* m_renderer;

public:
	enum DATA_USAGE { STATIC=0, DYNAMIC=1, DONTCARE=2 };

	DX12VertexBuffer(size_t size, DX12Renderer* device);
	virtual ~DX12VertexBuffer();
	virtual void setData(const void* data, size_t size, size_t offset);
	virtual void bind(size_t offset, size_t size, unsigned int location);
	virtual void unbind();
	virtual size_t getSize();

};

