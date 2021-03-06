﻿#include "DX12Renderer.h"

#include "../Technique.h"
#include "DX12Technique.h"
#include "DX12Material.h"
#include "DX12Mesh.h"
#include "DX12RenderState.h"
#include "DX12ConstantBuffer.h"
#include "DX12Texture2D.h"
#include "DX12VertexBuffer.h"

#include <stdio.h>
#include <cassert>
#include <guiddef.h>
//#include <pix3.h> // Used for pix events

#define MULTITHREADED
const UINT DX12Renderer::NUM_WORKER_THREADS = 4;
const UINT DX12Renderer::NUM_SWAP_BUFFERS = 2;
const UINT DX12Renderer::MAX_NUM_SAMPLERS = 1;

DX12Renderer::DX12Renderer()
	: m_renderTargetDescriptorSize(0U)
	, m_fenceValue(0U)
	, m_eventHandle(nullptr)
	, m_globalWireframeMode(false) 
	, m_firstFrame(true)
	, m_numSamplerDescriptors(0U)
	, m_samplerDescriptorHandleIncrementSize(0U)
{
	m_renderTargets.resize(NUM_SWAP_BUFFERS);
}

DX12Renderer::~DX12Renderer() {
	// Tell workers to shut down
	m_running.store(false);
	{
		// Wake up workers for termination
		std::lock_guard<std::mutex> guard(m_mainMutex);
		for (int i = 0; i < NUM_WORKER_THREADS; i++) {
			m_runWorkers[i] = true;
		}
	}
	m_mainCondVar.notify_all();

	waitForGPU();
	reportLiveObjects();

	for (std::thread& thread : m_workerThreads)
		thread.join();
}

int DX12Renderer::shutdown() {
	return 0;
}

Mesh* DX12Renderer::makeMesh() {
	return new DX12Mesh();
}

Texture2D* DX12Renderer::makeTexture2D() {
	return new DX12Texture2D(this);
}

Sampler2D* DX12Renderer::makeSampler2D() {
	assert(m_numSamplerDescriptors < MAX_NUM_SAMPLERS);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += m_samplerDescriptorHandleIncrementSize * m_numSamplerDescriptors;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_samplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += m_samplerDescriptorHandleIncrementSize * m_numSamplerDescriptors;

	m_numSamplerDescriptors++;

	return new DX12Sampler2D(m_device.Get(), cpuHandle, gpuHandle);
}

ConstantBuffer* DX12Renderer::makeConstantBuffer(std::string NAME, unsigned int location) {
	return new DX12ConstantBuffer(NAME, location, this);
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
	assert(m_firstFrame); // Should never be called after initialization
	// Returns the pre command list
	return m_preCommand.list.Get();
}

ID3D12RootSignature* DX12Renderer::getRootSignature() const {
	return m_rootSignature.Get();
}

ID3D12CommandAllocator* DX12Renderer::getCmdAllocator() const {
	// Returns the pre command allocator
	return m_preCommand.allocator.Get();
}

UINT DX12Renderer::getNumSwapBuffers() const {
	return NUM_SWAP_BUFFERS;
}

UINT DX12Renderer::getFrameIndex() const {
	return m_swapChain->GetCurrentBackBufferIndex();
}

ID3D12DescriptorHeap* DX12Renderer::getSamplerDescriptorHeap() const {
	return m_samplerDescriptorHeap.Get();
}

VertexBuffer* DX12Renderer::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage) {
	return new DX12VertexBuffer(size, usage, this);
};

Material* DX12Renderer::makeMaterial(const std::string& name) {
	return new DX12Material(name, this);
}

Technique* DX12Renderer::makeTechnique(Material* m, RenderState* r) {
	return new DX12Technique((DX12Material*)m, (DX12RenderState*)r, this);
}

RenderState* DX12Renderer::makeRenderState() {
	DX12RenderState* newRS = new DX12RenderState();
	newRS->setGlobalWireFrame(&m_globalWireframeMode);
	newRS->setWireFrame(false);
	return newRS;
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

	DWORD dxgiFactoryFlags = 0;
#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	wComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(true);
	}
	wComPtr<IDXGIInfoQueue> dxgiInfoQueue;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())))) {
		dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

		dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
		dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
	}
#endif
	ThrowIfFailed(
		CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf()))
	);

	m_window = std::make_unique<Win32Window>(GetModuleHandle(NULL), width, height, "");

	// 1. Initalize the window
	if (!m_window->initialize()) {
		OutputDebugString(L"\nFailed to initialize Win32Window\n");
		return 1;
	}

	// 2. Find comlient adapter and create device

	// dxgi1_6 is only needed for the initialization process using the adapter
	IDXGIFactory6* factory = nullptr;
	IDXGIAdapter1* adapter = nullptr;
	// Create a factory and iterate through all available adapters 
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
	for (UINT adapterIndex = 0;; ++adapterIndex) {
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter)) {
			break; // No more adapters to enumerate
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device4), nullptr))) {
			break;
		}

		SafeRelease(&adapter);
	}
	if (adapter) {
		// Create the actual device
		ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
		SafeRelease(&adapter);
	} else {
		// Create warp device if no adapter was found
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
	}


	// 3. Create command queue/allocator/list

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Create allocators
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_preCommand.allocator)));
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_postCommand.allocator)));
	// Create command lists
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_preCommand.allocator.Get(), nullptr, IID_PPV_ARGS(&m_preCommand.list)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_postCommand.allocator.Get(), nullptr, IID_PPV_ARGS(&m_postCommand.list)));

	// Command lists are created in the recording state. Since there is nothing to
	// record right now and the main loop expects it to be closed, we close them
	m_preCommand.list->Close();
	m_postCommand.list->Close();

	// 4. Create fence
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 1;
	// Create an event handle to use for GPU synchronization
	m_eventHandle = CreateEvent(0, false, false, 0);

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
			m_swapChain->Release();
		}
	}

	// No more factory using
	SafeRelease(&factory);

	// 6. Create render target descriptors

	// Create descriptor heap for samplers
	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
	samplerHeapDesc.NumDescriptors = MAX_NUM_SAMPLERS;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerDescriptorHeap)));
	m_samplerDescriptorHandleIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	// Create descriptor heap for render target views
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_renderTargetsHeap)));

	// Create resources for the render targets
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

	// Define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE descRangeSrv[1];
	descRangeSrv[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRangeSrv[0].NumDescriptors = 1;
	descRangeSrv[0].BaseShaderRegister = 0; // register bX
	descRangeSrv[0].RegisterSpace = 0; // register (bX,spaceY)
	descRangeSrv[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	D3D12_DESCRIPTOR_RANGE descRangeSampler[1];
	descRangeSampler[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	descRangeSampler[0].NumDescriptors = MAX_NUM_SAMPLERS;
	descRangeSampler[0].BaseShaderRegister = 0; // register bX
	descRangeSampler[0].RegisterSpace = 0; // register (bX,spaceY)
	descRangeSampler[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Create descriptor tables
	D3D12_ROOT_DESCRIPTOR_TABLE dtSrv;
	dtSrv.NumDescriptorRanges = ARRAYSIZE(descRangeSrv);
	dtSrv.pDescriptorRanges = descRangeSrv;

	D3D12_ROOT_DESCRIPTOR_TABLE dtSampler;
	dtSampler.NumDescriptorRanges = ARRAYSIZE(descRangeSampler);
	dtSampler.pDescriptorRanges = descRangeSampler;

	// Create root descriptors
	D3D12_ROOT_DESCRIPTOR rootDescCBV = {};
	rootDescCBV.ShaderRegister = TRANSLATION;
	rootDescCBV.RegisterSpace = 0;
	D3D12_ROOT_DESCRIPTOR rootDescCBV2 = {};
	rootDescCBV2.ShaderRegister = DIFFUSE_TINT;
	rootDescCBV2.RegisterSpace = 0;

	// Create root parameters
	D3D12_ROOT_PARAMETER rootParam[4];
	
	rootParam[CBV_TRANSLATION].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[CBV_TRANSLATION].Descriptor = rootDescCBV;
	rootParam[CBV_TRANSLATION].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	rootParam[CBV_DIFFUSE_TINT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[CBV_DIFFUSE_TINT].Descriptor = rootDescCBV2;
	rootParam[CBV_DIFFUSE_TINT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	rootParam[DT_SRVS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[DT_SRVS].DescriptorTable = dtSrv;
	rootParam[DT_SRVS].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[DT_SAMPLERS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[DT_SAMPLERS].DescriptorTable = dtSampler;
	rootParam[DT_SAMPLERS].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	// Serialize and create the actual signature
	ID3DBlob* sBlob;
	ID3DBlob* errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sBlob, &errorBlob);
	if (FAILED(hr)) {
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		ThrowIfFailed(hr);
	}
	ThrowIfFailed(m_device->CreateRootSignature(0, sBlob->GetBufferPointer(), sBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));


	// Reset the command list to prep for initialization commands
	ThrowIfFailed(m_preCommand.list->Reset(m_preCommand.allocator.Get(), nullptr));

	OutputDebugString(L"DX12 Initialized.\n");

	// Initialize worker threads
	m_workerThreads.reserve(NUM_WORKER_THREADS);
	m_runWorkers.resize(NUM_WORKER_THREADS);
	m_numWorkersFinished = 0;
	// Atomic boolean telling worker threads to run
	m_running.store(true);
	for (int i = 0; i < NUM_WORKER_THREADS; i++) {
		// Create command allocator and list
		m_workerCommands.push_back(Command());
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_workerCommands.back().allocator)));
		ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_workerCommands.back().allocator.Get(), nullptr, IID_PPV_ARGS(&m_workerCommands.back().list)));
		m_workerCommands.back().list->Close();
		m_runWorkers[i] = false;
		// Create thread
		m_workerThreads.emplace_back(&DX12Renderer::workerThread, this, i);
	}

	return 0;
}

/*
 Super simplified implementation of a work submission
 TODO.
*/

void DX12Renderer::submit(Mesh* mesh) {
	drawList[(DX12Technique*)mesh->technique].push_back((DX12Mesh*)mesh);
};

void DX12Renderer::workerThread(unsigned int id) {

	while (true) {
		{
			// Wait for event from main thread to begin
			std::unique_lock<std::mutex> mlock(m_mainMutex);
			m_mainCondVar.wait(mlock, [&]() { return m_runWorkers[id]; });
			m_runWorkers[id] = false;
		}
		// Stop thread if asked
		if (!m_running) break;

		auto& allocator = m_workerCommands[id].allocator;
		auto& list = m_workerCommands[id].list;

		// Reset
		allocator->Reset();
		list->Reset(allocator.Get(), nullptr);
		
		list->OMSetRenderTargets(1, &m_cdh, true, nullptr);
		list->SetGraphicsRootSignature(m_rootSignature.Get());

		// Iterate part of the drawlist and draw
		auto start = drawList.begin();
		unsigned int load = static_cast<unsigned int>(drawList.size()) / NUM_WORKER_THREADS;
		std::advance(start, id * load);
		auto end = start;
		if (id == NUM_WORKER_THREADS - 1) {
			end = drawList.end();
		} else {
			std::advance(end, load);
		}
		for (auto work = start; work != end; ++work) {
			// Set pipeline
			// TODO: only do this when neccesary
			list->SetPipelineState(work->first->getPipelineState());

			// Enable the technique
			// This binds constant buffers
			work->first->enable(this, list.Get()); 

			// Set topology
			list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//Set necessary states.
			list->RSSetViewports(1, &m_viewport);
			list->RSSetScissorRects(1, &m_scissorRect);

			for (auto mesh : work->second) {
				size_t numberElements = mesh->geometryBuffers[0].numElements;
				for (auto t : mesh->textures) {
					static_cast<DX12Texture2D*>(t.second)->bind(t.first, list.Get());
				}

				// Bind vertices, normals and UVs
				for (auto element : mesh->geometryBuffers) {
					mesh->bindIAVertexBuffer(element.first, list.Get());
				}
				// Bind translation constant buffer
				static_cast<DX12ConstantBuffer*>(mesh->txBuffer)->bind(work->first->getMaterial(), list.Get());
				// Draw
				list->DrawInstanced(static_cast<UINT>(numberElements), 1, 0, 0);
			}
		}
		list->Close();

		{
			// Notify main thread that rendering subtask is done
			std::lock_guard<std::mutex> guard(m_workerMutex);
			m_numWorkersFinished++;
		}
		m_workerCondVar.notify_all();
	}
}

/*
 Naive implementation, no re-ordering, checking for state changes, etc.
 TODO.
//*/
#ifdef MULTITHREADED
void DX12Renderer::frame() {

	if(m_firstFrame) {
		//Execute the initialization command list
		ThrowIfFailed(m_preCommand.list->Close());
		ID3D12CommandList* listsToExecute[] = { m_preCommand.list.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

		waitForGPU(); //Wait for GPU to finish.
		m_firstFrame = false;
	}

	// Get the handle for the current render target used as back buffer
	m_cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	m_cdh.ptr += m_renderTargetDescriptorSize * getFrameIndex();

	{
		// Notify workers to begin creating command lists
		std::lock_guard<std::mutex> guard(m_mainMutex);
		for (int i = 0; i < NUM_WORKER_THREADS; i++) {
			m_runWorkers[i] = true;
		}
	}
	m_mainCondVar.notify_all();

	// Command list allocators can only be reset when the associated command lists have
	// finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	m_preCommand.allocator->Reset();
	m_preCommand.list->Reset(m_preCommand.allocator.Get(), nullptr);
	
	// Indicate that the back buffer will be used as render target.
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = m_renderTargets[getFrameIndex()].Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_preCommand.list->ResourceBarrier(1, &barrierDesc);

	m_preCommand.list->OMSetRenderTargets(1, &m_cdh, true, nullptr);
	m_preCommand.list->ClearRenderTargetView(m_cdh, m_clearColor, 0, nullptr);

	{
		//Execute the pre command list
		m_preCommand.list->Close();
		ID3D12CommandList* listsToExecute[] = { m_preCommand.list.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);
	}

	// Wait for worker threads to finish
	{
		std::unique_lock<std::mutex> mlock(m_workerMutex);
		//OutputDebugStringA("wait\n");
		m_workerCondVar.wait(mlock, [&]() { return m_numWorkersFinished >= NUM_WORKER_THREADS; });
		//OutputDebugStringA("do\n");
		//OutputDebugStringA("All workers finished! Executing worker command lists\n");
		m_numWorkersFinished.store(0);
		// Execute the worker command lists
		ID3D12CommandList* listsToExecute[NUM_WORKER_THREADS];
		//listsToExecute[0] = m_preCommand.list.Get();
		for (int i = 0; i < NUM_WORKER_THREADS; i++) {
			listsToExecute[i] = m_workerCommands[i].list.Get();
		}
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);
		// Clear submitted draw list
		drawList.clear();
	}

	// Reset post command
	m_postCommand.allocator->Reset();
	m_postCommand.list->Reset(m_postCommand.allocator.Get(), nullptr);

	// Indicate that the back buffer will now be used to present
	barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = m_renderTargets[getFrameIndex()].Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_postCommand.list->ResourceBarrier(1, &barrierDesc);

	{
		// Close the list to prepare it for execution.
		m_postCommand.list->Close();
		ID3D12CommandList* listsToExecute[] = { m_postCommand.list.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);
	}

};
#else
void DX12Renderer::frame() {

	if (m_firstFrame) {
		//Execute the initialization command list
		ThrowIfFailed(m_preCommand.list->Close());
		ID3D12CommandList* listsToExecute[] = { m_preCommand.list.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

		waitForGPU(); //Wait for GPU to finish.
		m_firstFrame = false;
	}

	// Command list allocators can only be reset when the associated command lists have
	// finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	m_preCommand.allocator->Reset();
	
	m_preCommand.list->Reset(m_preCommand.allocator.Get(), nullptr);
	
	// Indicate that the back buffer will be used as render target.
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = m_renderTargets[getFrameIndex()].Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_preCommand.list->ResourceBarrier(1, &barrierDesc);

	// Record commands
	// Get the handle for the current render target used as back buffer
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += m_renderTargetDescriptorSize * getFrameIndex();

	m_preCommand.list->OMSetRenderTargets(1, &cdh, true, nullptr);
	m_preCommand.list->ClearRenderTargetView(cdh, m_clearColor, 0, nullptr);

	// Set root signature
	m_preCommand.list->SetGraphicsRootSignature(m_rootSignature.Get());

	for (auto work : drawList) {
		// Set pipeline
		// TODO: only do this when neccesary
		m_preCommand.list->SetPipelineState(work.first->getPipelineState());

		// Enable the technique
		// This binds constant buffers
		work.first->enable(this, m_preCommand.list.Get());

		// Set topology
		m_preCommand.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//Set necessary states.
		m_preCommand.list->RSSetViewports(1, &m_viewport);
		m_preCommand.list->RSSetScissorRects(1, &m_scissorRect);

		for (auto mesh : work.second) {
			size_t numberElements = mesh->geometryBuffers[0].numElements;
			for (auto t : mesh->textures) {
				static_cast<DX12Texture2D*>(t.second)->bind(t.first, m_preCommand.list.Get());
			}

			// Bind vertices, normals and UVs
			for (auto element : mesh->geometryBuffers) {
				mesh->bindIAVertexBuffer(element.first, m_preCommand.list.Get());
			}
			// Bind translation constant buffer
			static_cast<DX12ConstantBuffer*>(mesh->txBuffer)->bind(work.first->getMaterial(), m_preCommand.list.Get());
			// Draw
			m_preCommand.list->DrawInstanced(static_cast<UINT>(numberElements), 1, 0, 0);
		}

	}
	drawList.clear();

	// Indicate that the back buffer will now be used to present
	barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = m_renderTargets[getFrameIndex()].Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_preCommand.list->ResourceBarrier(1, &barrierDesc);

	//Close the list to prepare it for execution.
	m_preCommand.list->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { m_preCommand.list.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

};
#endif
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

void DX12Renderer::reportLiveObjects() {
#ifdef _DEBUG
	OutputDebugStringA("== REPORT LIVE OBJECTS ==\n");
	wComPtr<IDXGIDebug1> dxgiDebug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
		ThrowIfFailed(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL)));
	}
	OutputDebugStringA("=========================\n");
#endif
}

void DX12Renderer::setClearColor(float r, float g, float b, float a) {
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
};

void DX12Renderer::clearBuffer(unsigned int flag) {
	// TODO?
};

void DX12Renderer::setRenderState(RenderState* ps) {
	// doesnt do anything
	ps->set();
};
