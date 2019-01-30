#pragma once
#include <unordered_map>
#include "../Mesh.h"

class DX12Mesh : public Mesh {
public:
	DX12Mesh();
	~DX12Mesh();
	void addIAVertexBufferBinding(VertexBuffer* buffer, size_t offset, size_t numElements, size_t sizeElement, unsigned int inputStream) override;
};

