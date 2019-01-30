#include "DX12ConstantBuffer.h"
#include "DX12Renderer.h"
#include "DX12Material.h"

DX12ConstantBuffer::DX12ConstantBuffer(std::string name, unsigned int location, DX12Renderer* renderer) 
	: m_name(name)
	, m_renderer(renderer)
{

	// Create a descriptor heap to store the constant buffer descriptor.
	// It is possible to store all descriptors in one heap (this could be moved out from this class)

	m_mainDescriptorHeap = new wComPtr<ID3D12DescriptorHeap>[renderer->getNumSwapBuffers()];
	m_cbGPUAddress = new UINT8*[renderer->getNumSwapBuffers()];
	
	for (UINT i = 0; i < renderer->getNumSwapBuffers(); ++i) {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		ThrowIfFailed(renderer->getDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_mainDescriptorHeap[i])));
	}

	// Create an upload heap to hold the constant buffer
	// create a resource heap, descriptor heap, and pointer to cbv for each frame
	for (UINT i = 0; i < renderer->getNumSwapBuffers(); ++i) {
		ThrowIfFailed(renderer->getDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&m_constantBufferUploadHeap)));
		m_constantBufferUploadHeap->SetName(L"Constant Buffer Upload Resource Heap");

		// TODO: copy upload heap to a default heap when the data is not often changed

	}
}

DX12ConstantBuffer::~DX12ConstantBuffer() {

	delete[] m_mainDescriptorHeap;
	delete[] m_cbGPUAddress;

	/*if (handle != 0) {
		glDeleteBuffers(1, &handle);
		handle = 0;
	};*/
}

// this allows us to not know in advance the type of the receiving end, vec3, vec4, etc.
void DX12ConstantBuffer::setData(const void* data, size_t size, Material* m, unsigned int location) {
	
	for (UINT i = 0; i < m_renderer->getNumSwapBuffers(); ++i) {
		// Create a constant buffer view which descriped the buffer and contains a pointer to the memory where the data resides
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBufferUploadHeap->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (size + 255) & ~255; // Required 256-byte alignment
		m_renderer->getDevice()->CreateConstantBufferView(&cbvDesc, m_mainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());

		// Map the constant buffer
		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
		ThrowIfFailed(m_constantBufferUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&m_cbGPUAddress[i])));
		memcpy(m_cbGPUAddress[i], data, size);
	}

}

void DX12ConstantBuffer::bind(Material* m) {
	
	// set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_mainDescriptorHeap[m_renderer->getFrameIndex()].Get() };
	m_renderer->getCmdList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the root descriptor table 0 to the constant buffer descriptor heap
	m_renderer->getCmdList()->SetGraphicsRootDescriptorTable(0, m_mainDescriptorHeap[m_renderer->getFrameIndex()]->GetGPUDescriptorHandleForHeapStart());

}
