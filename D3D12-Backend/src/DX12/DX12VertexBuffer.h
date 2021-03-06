#pragma once

#include <vector>

#include "../VertexBuffer.h"
#include "DX12.h"

class DX12Renderer;

class DX12VertexBuffer : public VertexBuffer {
public:
	enum DATA_USAGE { STATIC = 0, DYNAMIC = 1, DONTCARE = 2 };

	DX12VertexBuffer(size_t byteSize, VertexBuffer::DATA_USAGE usage, DX12Renderer* renderer);
	virtual ~DX12VertexBuffer();
	virtual void setData(const void* data, size_t size, size_t offset) override;
	virtual void bind(size_t offset, size_t size, unsigned int location) override;
	void bind(size_t offset, size_t size, unsigned int location, ID3D12GraphicsCommandList3* cmdList);
	virtual void unbind() override;
	virtual size_t getSize() override;

	void setVertexSize(size_t size);

	void releaseBufferedObjects(); 

private:
	VertexBuffer::DATA_USAGE m_usage;

	wComPtr<ID3D12Resource> m_vertexBuffer;

	// uploadBuffers to be released
	std::vector<ID3D12Resource*> m_uploadBuffers;

	size_t m_byteSize;
	size_t m_vertexSize;

	UINT m_lastBoundVBSlot;

	DX12Renderer* m_renderer;

};

