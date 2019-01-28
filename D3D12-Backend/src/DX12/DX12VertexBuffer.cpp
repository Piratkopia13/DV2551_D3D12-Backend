#include "DX12VertexBuffer.h"

#include <cassert>

#include "DX12Renderer.h"

DX12VertexBuffer::DX12VertexBuffer(size_t size, DX12Renderer* renderer) {
	/*// Heap properties of the resource
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

	device->CreateCommittedResource(
		&hp, 
		D3D12_HEAP_FLAG_NONE, 
		&rd,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));
	
	m_vertexBuffer->SetName(L"vb heap");*/

	m_renderer = renderer;
	m_size = size;
}

DX12VertexBuffer::~DX12VertexBuffer() {

}

void DX12VertexBuffer::setData(const void * data, size_t size, size_t offset) {
	/*assert(size + offset <= m_size);

	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 }; // No intention of reading this resource on the CPU.
	m_vertexBuffer->Map(0, &range, &dataBegin);
	memcpy_s(dataBegin, size, data, size);
	m_vertexBuffer->Unmap(0, nullptr);*/
}

void DX12VertexBuffer::bind(size_t offset, size_t size, unsigned int location) {
}

void DX12VertexBuffer::unbind() {
}

size_t DX12VertexBuffer::getSize() {
	return size_t(0);
}
