#pragma once
#include <GL/glew.h>
#include "../ConstantBuffer.h"

class DX12ConstantBuffer : public ConstantBuffer
{
public:
	DX12ConstantBuffer(std::string NAME, unsigned int location);
	~DX12ConstantBuffer();
	void setData(const void* data, size_t size, Material* m, unsigned int location);
	void bind(Material*);

private:

	std::string name;
	GLuint location;
	GLuint handle;
	GLuint index;
	void* buff = nullptr;
	void* lastMat;
};

