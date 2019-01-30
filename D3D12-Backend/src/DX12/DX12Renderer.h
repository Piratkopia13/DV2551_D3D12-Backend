#pragma once

#include "../Renderer.h"
#include "DX12.h"
#include "Win32Window.h"
#include <memory>
#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DX12Mesh;
class DX12Technique;

class DX12Renderer : public Renderer {
public:
	DX12Renderer();
	~DX12Renderer();

	Material* makeMaterial(const std::string& name);
	Mesh* makeMesh();
	//VertexBuffer* makeVertexBuffer();
	VertexBuffer* makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage);
	ConstantBuffer* makeConstantBuffer(std::string NAME, unsigned int location);
	//	ResourceBinding* makeResourceBinding();
	RenderState* makeRenderState();
	Technique* makeTechnique(Material* m, RenderState* r);
	Texture2D* makeTexture2D();
	Sampler2D* makeSampler2D();

	std::string getShaderPath();
	std::string getShaderExtension();
	ID3D12Device4* getDevice() const;
	ID3D12CommandQueue* getCmdQueue() const;
	ID3D12GraphicsCommandList3* getCmdList() const;
	ID3D12RootSignature* getRootSignature() const;
	ID3D12CommandAllocator* getCmdAllocator() const;
	UINT getNumSwapBuffers() const;
	UINT getFrameIndex() const;

	int initialize(unsigned int width = 640, unsigned int height = 480);
	void setWinTitle(const char* title);
	int shutdown();

	void setClearColor(float, float, float, float);
	void clearBuffer(unsigned int);
	//	void setRenderTarget(RenderTarget* rt); // complete parameters
	void setRenderState(RenderState* ps);
	void submit(Mesh* mesh);
	void frame();
	void present();
	
	void waitForGPU();

protected:
	

private:
	std::unique_ptr<Win32Window> m_window;
	bool m_globalWireframeMode;
	float m_clearColor[4];
	bool m_firstFrame;
	
	static const UINT NUM_SWAP_BUFFERS = 2;
	UINT m_frameIndex;

	// DX12 stuff
	Microsoft::WRL::ComPtr<ID3D12Device4> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator; // Allocator only grows, use multple (one for each thing)
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList3> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12Fence1> m_fence;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_renderTargetsHeap;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource1> m_renderTargets[NUM_SWAP_BUFFERS];
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	UINT m_renderTargetDescriptorSize;
	UINT64 m_fenceValue;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	HANDLE m_eventHandle;

	// Upload buffer stuff
	// Currently only one large
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
	size_t m_uploadHeapSize;
	UINT8* m_pDataBegin = nullptr; // Starting position of upload buffer
	UINT8* m_pDataCur = nullptr; // Current position of upload buffer
	UINT8* m_pDataEnd = nullptr; // End position of upload buffer

	//std::vector<Mesh*> drawList;
	std::unordered_map<DX12Technique*, std::vector<DX12Mesh*>> drawList2;

	//bool globalWireframeMode = false;

	////int initializeDX12(int major, int minor, unsigned int width, unsigned int height);
	//float clearColor[4] = { 0,0,0,0 };
	//std::unordered_map<int, int> BUFFER_MAP = {
	//	{0, 0},
	//	{CLEAR_BUFFER_FLAGS::COLOR, GL_COLOR_BUFFER_BIT },
	//	{CLEAR_BUFFER_FLAGS::DEPTH, GL_DEPTH_BUFFER_BIT },
	//	{CLEAR_BUFFER_FLAGS::STENCIL, GL_STENCIL_BUFFER_BIT }
	//};
};

