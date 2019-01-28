#include <stdio.h>
#include "DX12Renderer.h"
//#include <GL/glew.h>

//#include "MaterialGL.h"
//#include "MeshGL.h"
#include "../Mesh.h"
#include "../Technique.h"
//#include "ResourceBindingGL.h"
//#include "RenderStateGL.h"
//#include "VertexBufferGL.h"
//#include "ConstantBufferGL.h"
//#include "Texture2DGL.h"

DX12Renderer::DX12Renderer() {
}

DX12Renderer::~DX12Renderer() {
}

int DX12Renderer::shutdown() {
	/*SDL_GL_DeleteContext(context);
	SDL_Quit();*/
	return 0;
}

Mesh* DX12Renderer::makeMesh() {
	//return new MeshGL();
	return nullptr;
}

Texture2D* DX12Renderer::makeTexture2D() {
	//return (Texture2D*)new Texture2DGL();
	return nullptr;
}

Sampler2D* DX12Renderer::makeSampler2D() {
	//return (Sampler2D*)new Sampler2DGL();
	return nullptr;
}

ConstantBuffer* DX12Renderer::makeConstantBuffer(std::string NAME, unsigned int location) {
	//return new ConstantBufferGL(NAME, location);
	return nullptr;
}

std::string DX12Renderer::getShaderPath() {
	//return std::string("..\\assets\\GL45\\");
	return "";
}

std::string DX12Renderer::getShaderExtension() {
	//return std::string(".glsl");
	return "";
}

VertexBuffer* DX12Renderer::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage) {
	//return new VertexBufferGL(size, usage);
	return nullptr;
};

Material* DX12Renderer::makeMaterial(const std::string& name) {
	//return new MaterialGL(name);
	return nullptr;
}

Technique* DX12Renderer::makeTechnique(Material* m, RenderState* r) {
	Technique* t = new Technique(m, r);
	return t;
}

RenderState* DX12Renderer::makeRenderState() {
	/*RenderStateGL* newRS = new RenderStateGL();
	newRS->setGlobalWireFrame(&this->globalWireframeMode);
	newRS->setWireFrame(false);
	return (RenderState*)newRS;*/
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
	// DXGIFactory



	// Initialize DirectX12
	if (adapter) {
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device));
		adapter->Release();
	}

	// 3. Create command queue/allocator/list
	// At least one allocator/list per thread and one per 

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	//queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	// Create allocator
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandQueue)));
	// Create command list
	//ThrowIfFailed(m_device->CreateCommandList(asd, dasd, IID_PPV_ARGS(&m_commandQueue)));

	// 4. Create fence
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 1;
	m_eventHandle = CreateEvent(0, false, false, 0);


	// 5. Create swap chain
	//factory->CreateSwapChainForHwnd(m_commandQueue, wndHandle, &scDesc, nullptr, nullptr, reinterpret_cast<ID

	// 6. Create render target descriptors

	const UINT NUM_SWAP_BUFFERS = 2;
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_renderTargetsHeap)));
	// Create resources for the render target
	m_renderTargetDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	// One RTV for each frame
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++) {
		// blah
		hr = m_swapChain->GetBuffer(blah);
		m_device->CreateRenderTargetView(blah);
	}

	// 8. Create root signature
	// CANCER

	// Other classes
	{
		// 10. Create pipeline state

		// 12. Create vertex buffer resources

		// 13. Draaaaaaw
	}




#if	defined(DEBUG)	||	defined(_DEBUG)	//	Enable	the	D3D12	debug	layer. 
	{		
		ComPtr<ID3D12Debug>	debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController->EnableDebugLayer();
	} 
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory		//	Try	to	create	hardware	device. HRESULT	hardwareResult	=	D3D12CreateDevice(		nullptr,							//	default	adapter
												  D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice));

	//	Fallback	to	WARP	device. 
	if(FAILED(hardwareResult)) {
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mdxgiFactory>EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice)));
	}



	//if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
	//	fprintf(stderr, "%s", SDL_GetError());
	//	exit(-1);
	//}
	//// Request an DX12 4.5 context (should be core)
	//SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	//// Also request a depth buffer
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	//window = SDL_CreateWindow("DX12", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_DX12);
	//context = SDL_GL_CreateContext(window);

	//SDL_GL_MakeCurrent(window, context);

	//int major, minor;
	//SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	//SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

	//glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	//glClearDepth(1.0f);
	//glDepthFunc(GL_LEQUAL);

	//glViewport(0, 0, width, height);

	//glewExperimental = GL_TRUE;
	//GLenum err = glewInit();
	//if (GLEW_OK != err) {
	//	fprintf(stderr, "Error GLEW: %s\n", glewGetErrorString(err));
	//}

	return 0;
}

/*
 Super simplified implementation of a work submission
 TODO.
*/

//int perMat = 1;
void DX12Renderer::submit(Mesh* mesh) {
	/*if (perMat) {
		drawList2[mesh->technique].push_back(mesh);
	} else
		drawList.push_back(mesh);*/
};

/*
 Naive implementation, no re-ordering, checking for state changes, etc.
 TODO.
*/
void DX12Renderer::frame() {
	//if (perMat != 1) {

	//	for (auto mesh : drawList) {
	//		mesh->technique->enable(this);
	//		size_t numberElements = mesh->geometryBuffers[0].numElements;
	//		glBindTexture(GL_TEXTURE_2D, 0);
	//		for (auto t : mesh->textures) {
	//			// we do not really know here if the sampler has been
	//			// defined in the shader.
	//			t.second->bind(t.first);
	//		}
	//		for (auto element : mesh->geometryBuffers) {
	//			mesh->bindIAVertexBuffer(element.first);
	//		}
	//		mesh->txBuffer->bind(mesh->technique->getMaterial());
	//		glDrawArrays(GL_TRIANGLES, 0, numberElements);
	//	}
	//	drawList.clear();
	//} else {
	//	for (auto work : drawList2) {
	//		work.first->enable(this);
	//		for (auto mesh : work.second) {
	//			size_t numberElements = mesh->geometryBuffers[0].numElements;
	//			glBindTexture(GL_TEXTURE_2D, 0);
	//			for (auto t : mesh->textures) {
	//				// we do not really know here if the sampler has been
	//				// defined in the shader.
	//				t.second->bind(t.first);
	//			}
	//			for (auto element : mesh->geometryBuffers) {
	//				mesh->bindIAVertexBuffer(element.first);
	//			}
	//			mesh->txBuffer->bind(work.first->getMaterial());
	//			glDrawArrays(GL_TRIANGLES, 0, numberElements);
	//		}
	//	}
	//	drawList2.clear();
	//}
};

void DX12Renderer::present() {
	//SDL_GL_SwapWindow(window);
}

void DX12Renderer::setClearColor(float r, float g, float b, float a) {
	//glClearColor(r, g, b, a);
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
	//// naive implementation
	//ps->set();
};
