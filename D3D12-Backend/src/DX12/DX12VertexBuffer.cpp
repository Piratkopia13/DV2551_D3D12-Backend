#include "DX12VertexBuffer.h"

DX12VertexBuffer::DX12VertexBuffer(size_t size, ID3D12Device4) {
}

DX12VertexBuffer::~DX12VertexBuffer() {

}

void DX12VertexBuffer::setData(const void * data, size_t size, size_t offset) {
	// Heap properties of the resource
	D3D12_HEAP_PROPERTIES hp = {};
	// Default heap, no CPU access, usually populated through upload heaps
	hp.Type = D3D12_HEAP_TYPE_DEFAULT;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	// Resource description
	D3D12_RESOURCE_DESC rd = {};
	// Declare that the resource is used as a buffer
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = size;
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 }; // No intention of reading this resource on the CPU.
	m_vertexBuffer->Map(0, &range, &dataBegin);
	memcpy_s(dataBegin, size, data, size);
	m_vertexBuffer->Unmap(0, nullptr);
}

void DX12VertexBuffer::bind(size_t offset, size_t size, unsigned int location) {
}

void DX12VertexBuffer::unbind() {
}

size_t DX12VertexBuffer::getSize() {
	return size_t();
}
