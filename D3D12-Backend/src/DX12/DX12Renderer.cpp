#include "DX12Renderer.h"

#include <stdio.h>

#include "DX12Material.h"
#include "DX12Mesh.h"
#include "../Technique.h"
#include "DX12RenderState.h"
#include "DX12Technique.h"
#include "DX12ConstantBuffer.h"
#include "DX12Texture2D.h"
#include "DX12VertexBuffer.h"

DX12Renderer::DX12Renderer()
	: m_renderTargetDescriptorSize(0)
	, m_fenceValue(0)
	, m_eventHandle(nullptr)
	, m_globalWireframeMode(false) 
	, m_frameIndex(0)
	, m_firstFrame(true)
{
}

DX12Renderer::~DX12Renderer() {
}

int DX12Renderer::shutdown() {
	/*SDL_GL_DeleteContext(context);
	SDL_Quit();*/
	return 0;
}

Mesh* DX12Renderer::makeMesh() {
	return new DX12Mesh();
}

Texture2D* DX12Renderer::makeTexture2D() {
	return (Texture2D*)new DX12Texture2D(this);
}

Sampler2D* DX12Renderer::makeSampler2D() {
	//return (Sampler2D*)new Sampler2DGL();
	return nullptr;
}

ConstantBuffer* DX12Renderer::makeConstantBuffer(std::string NAME, unsigned int location) {
	return new DX12ConstantBuffer(NAME, location, this);
	return nullptr;
}

std::string DX12Renderer::getShaderPath() {
	return std::string("..\\assets\\DX12\\");
}

std::string DX12Renderer::getShaderExtension() {
	return std::string(".hlsl");
}

ID3D12Device4* DX12Renderer::getDevice() const {
	return m_device.Get();
}

ID3D12CommandQueue* DX12Renderer::getCmdQueue() const {
	return m_commandQueue.Get();
}

ID3D12GraphicsCommandList3* DX12Renderer::getCmdList() const {
	return m_commandList.Get();
}

ID3D12RootSignature* DX12Renderer::getRootSignature() const {
	return m_rootSignature.Get();
}

ID3D12CommandAllocator* DX12Renderer::getCmdAllocator() const {
	return m_commandAllocator.Get();
}

UINT DX12Renderer::getNumSwapBuffers() const {
	return NUM_SWAP_BUFFERS;
}

UINT DX12Renderer::getFrameIndex() const {
	return m_swapChain->GetCurrentBackBufferIndex();
}

VertexBuffer* DX12Renderer::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage) {
	// Temp size
	return new DX12VertexBuffer(size, sizeof(float) * 8, this);
};

Material* DX12Renderer::makeMaterial(const std::string& name) {
	return new DX12Material(name, this);
}

Technique* DX12Renderer::makeTechnique(Material* m, RenderState* r) {
	DX12Technique* t = new DX12Technique((DX12Material*)m, (DX12RenderState*)r, this);
	return t;
}

RenderState* DX12Renderer::makeRenderState() {
	DX12RenderState* newRS = new DX12RenderState();
	newRS->setGlobalWireFrame(&m_globalWireframeMode);
	newRS->setWireFrame(false);
	return (RenderState*)newRS;
	return nullptr;
}

void DX12Renderer::setWinTitle(const char* title) {
	size_t size = strlen(title) + 1;
	wchar_t* wtext = new wchar_t[size];

	size_t outSize;
	mbstowcs_s(&outSize, wtext, size, title, size - 1);

	m_window->setWindowTitle(wtext);
	delete[] wtext;
}

int DX12Renderer::initialize(unsigned int width, unsigned int height) {

#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	ID3D12Debug1* debugController = nullptr;

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(true);
	}
	//SafeRelease(&debugController);
#endif

	//GetWindowLong(&hwnd, 0);
	//GetModuleHandle(NULL);

	m_window = std::make_unique<Win32Window>(GetModuleHandle(NULL), width, height, "");

	// Initalize the window
	if (!m_window->initialize()) {
		OutputDebugString(L"\nFailed to initialize Win32Window\n");
		//Logger::Error("Failed to initialize Win32Window!");
		return 1;
	}

	// 1. Find comlient adapter

	// Initialize DirectX12
	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIFactory6* factory = nullptr;
	IDXGIAdapter1* adapter = nullptr;
	//First a factory is created to iterate through the adapters available.
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
	for (UINT adapterIndex = 0;; ++adapterIndex) {
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter)) {
			break; //No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device4), nullptr))) {
			break;
		}
		//ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device5), nullptr));

		SafeRelease(&adapter);
	}
	if (adapter) {
		//Create the actual device.
		ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
		SafeRelease(&adapter);
	} else {
		// Create warp device if no adapter was found.
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
	}



	// 3. Create command queue/allocator/list
	// At least one allocator/list per thread and one per 

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Create allocator
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	// Create command list
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	m_commandList->Close();

	// 5. Create swap chain
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = NUM_SWAP_BUFFERS;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;
	if (SUCCEEDED(factory->CreateSwapChainForHwnd(m_commandQueue.Get(), *m_window->getHwnd(), &scDesc, nullptr, nullptr, &swapChain1))) {
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain)))) {
			// WHY THO?
			m_swapChain->Release();
		}
	}

	// No more factory using
	SafeRelease(&factory);

	// 4. Create fence
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	m_eventHandle = CreateEvent(0, false, false, 0);

	// 6. Create render target descriptors

	// Create descriptor heap for render target views
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_renderTargetsHeap)));

	//Create resources for the render targets.
	m_renderTargetDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	// One RTV for each frame
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++) {
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, cdh);
		cdh.ptr += m_renderTargetDescriptorSize;
	}

	// 7. Viewport and scissor rect
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = (float)width;
	m_viewport.Height = (float)height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect.left = (long)0;
	m_scissorRect.right = (long)width;
	m_scissorRect.top = (long)0;
	m_scissorRect.bottom = (long)height;

	// 8. Create root signature

	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE dtRanges[1];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[0].NumDescriptors = 2; // two CBs
	dtRanges[0].BaseShaderRegister = 0; // register b0 (and b1)
	dtRanges[0].RegisterSpace = 0; // register (b0,space0) and (b1,space0)
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	//create root parameter
	D3D12_ROOT_PARAMETER rootParam[1];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	ID3DBlob* sBlob;
	ThrowIfFailed(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sBlob, nullptr));
	ThrowIfFailed(m_device->CreateRootSignature(0, sBlob->GetBufferPointer(), sBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));


	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));


	// Other classes
	{
		// 10. Create pipeline state

		// 12. Create vertex buffer resources


		// 13. Draaaaaaw
	}

	OutputDebugString(L"DID STUFF");

	return 0;
}

/*
 Super simplified implementation of a work submission
 TODO.
*/

int perMat = 1;
void DX12Renderer::submit(Mesh* mesh) {
	if (perMat) {
		drawList2[(DX12Technique*)mesh->technique].push_back((DX12Mesh*)mesh);
	}/* else
		drawList.push_back(mesh);*/
};

/*
 Naive implementation, no re-ordering, checking for state changes, etc.
 TODO.
*/
void DX12Renderer::frame() {

	if(m_firstFrame) {
		//Execute the initialization command list
		ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* listsToExecute[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

		waitForGPU(); //Wait for GPU to finish.
		m_firstFrame = false;
	}

	// Command list allocators can only be reset when the associated command lists have
	// finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	m_commandAllocator->Reset();

	//if (perMat != 1) {

	//	//for (auto mesh : drawList) {
	//	//	mesh->technique->enable(this);
	//	//	size_t numberElements = mesh->geometryBuffers[0].numElements;
	//	//	//glBindTexture(GL_TEXTURE_2D, 0);
	//	//	for (auto t : mesh->textures) {
	//	//		// we do not really know here if the sampler has been
	//	//		// defined in the shader.
	//	//		t.second->bind(t.first);
	//	//	}
	//	//	for (auto element : mesh->geometryBuffers) {
	//	//		mesh->bindIAVertexBuffer(element.first);
	//	//	}
	//	//	mesh->txBuffer->bind(mesh->technique->getMaterial());
	//	//	//glDrawArrays(GL_TRIANGLES, 0, numberElements);
	//	//}
	//	//drawList.clear();
	//} else {
	//	for (auto work : drawList2) {

	auto work = *drawList2.begin();

	m_commandList->Reset(m_commandAllocator.Get(), work.first->getPipelineState());

	// Indicate that the back buffer will be used as render target.
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = m_renderTargets[getFrameIndex()].Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_commandList->ResourceBarrier(1, &barrierDesc);

	// Record commands
	// Get the handle for the current render target used as back buffer
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += m_renderTargetDescriptorSize * getFrameIndex();

	m_commandList->OMSetRenderTargets(1, &cdh, true, nullptr);

	m_commandList->ClearRenderTargetView(cdh, m_clearColor, 0, nullptr);

		// Set root signature
		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		// Enable the technique
		work.first->enable(this);

		// Set topology
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//Set necessary states.
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);


		for (auto mesh : work.second) {
			size_t numberElements = mesh->geometryBuffers[0].numElements;
			//glBindTexture(GL_TEXTURE_2D, 0);
			for (auto t : mesh->textures) {
				// we do not really know here if the sampler has been
				// defined in the shader.
				t.second->bind(t.first);
			}

			for (auto element : mesh->geometryBuffers) {
				mesh->bindIAVertexBuffer(element.first);
			}
			//mesh->txBuffer->bind(work.first->getMaterial());
			m_commandList->DrawInstanced(numberElements, 1, 0, 0);
			//glDrawArrays(GL_TRIANGLES, 0, numberElements);
		}

		//Indicate that the back buffer will now be used to present.
		barrierDesc = {};
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = m_renderTargets[getFrameIndex()].Get();
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		m_commandList->ResourceBarrier(1, &barrierDesc);

		//Close the list to prepare it for execution.
		m_commandList->Close();


		//}
		drawList2.clear();
	//}


	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

};

void DX12Renderer::present() {

	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	m_swapChain->Present1(0, 0, &pp);

	waitForGPU(); //Wait for GPU to finish.
				  //NOT BEST PRACTICE, only used as such for simplicity.
}

void DX12Renderer::waitForGPU() {
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//This is code implemented as such for simplicity. The cpu could for example be used
	//for other tasks to prepare the next frame while the current one is being rendered.

	//Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), fence);
	m_fenceValue++;

	//Wait until command queue is done.
	if (m_fence->GetCompletedValue() < fence) {
		m_fence->SetEventOnCompletion(fence, m_eventHandle);
		WaitForSingleObject(m_eventHandle, INFINITE);
	}
}

void DX12Renderer::setClearColor(float r, float g, float b, float a) {
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
};

void DX12Renderer::clearBuffer(unsigned int flag) {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//return;
	//// using is only valid inside the function!
	//using namespace CLEAR_BUFFER_FLAGS;
	//GLuint glFlags = BUFFER_MAP[flag & COLOR] | BUFFER_MAP[flag & DEPTH] | BUFFER_MAP[flag & STENCIL];
	//glClear(glFlags);
};

//void DX12Renderer::setRenderTarget(RenderTarget* rt) {};

void DX12Renderer::setRenderState(RenderState* ps) {
	// naive implementation
	ps->set();
};
