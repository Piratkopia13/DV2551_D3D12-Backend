#pragma once
#include <unordered_map>
#include "IA.h"
#include "VertexBuffer.h"
#include "Technique.h"
#include "Transform.h"
#include "ConstantBuffer.h"
#include "Texture2D.h"

class Mesh
{
public:
	Mesh();
	~Mesh();

	// technique has: Material, RenderState, Attachments (color, depth, etc)
	Technique* technique; 

	// translation buffers
	ConstantBuffer* txBuffer;
	// local copy of the translation
	Transform* transform;

	struct VertexBufferBind {
		size_t sizeElement, numElements, offset;
		VertexBuffer* buffer;
	};
	
	void addTexture(Texture2D* texture, unsigned int slot);

	// array of buffers with locations (binding points in shaders)
	virtual void addIAVertexBufferBinding(
		VertexBuffer* buffer, 
		size_t offset, 
		size_t numElements, 
		size_t sizeElement, 
		unsigned int inputStream);

	virtual void bindIAVertexBuffer(unsigned int location);
	std::unordered_map<unsigned int, VertexBufferBind> geometryBuffers;
	std::unordered_map<unsigned int, Texture2D*> textures;
};
