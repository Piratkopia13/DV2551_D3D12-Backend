#include "DX12Mesh.h"
#include "DX12VertexBuffer.h"

// Mesh::DATA_USAGE is an enum { STATIC, DYNAMIC, DONTCARE };

DX12Mesh::DX12Mesh() {}
DX12Mesh::~DX12Mesh() {}

void DX12Mesh::addIAVertexBufferBinding( VertexBuffer* buffer, size_t offset, size_t numElements, size_t sizeElement, unsigned int inputStream) {
	Mesh::addIAVertexBufferBinding(buffer, offset, numElements, sizeElement, inputStream);
	static_cast<DX12VertexBuffer*>(buffer)->setVertexSize(sizeElement);
};