#include <iostream>
#include "DX12Technique.h"
#include "DX12Renderer.h"

DX12Technique::DX12Technique(DX12Material* m, DX12RenderState* r, DX12Renderer* renderer)
	: Technique(m, r) {

	////// Pipline State //////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	ID3DBlob* vertexBlob = m->getShaderBlob(Material::ShaderType::VS);
	ID3DBlob* pixelBlob = m->getShaderBlob(Material::ShaderType::PS);

	//Specify pipeline stages:
	gpsd.pRootSignature = renderer->getRootSignature();
	gpsd.InputLayout = m->getInputLayoutDesc();
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vertexBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = vertexBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(pixelBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = pixelBlob->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.FillMode = (r->wireframeEnabled()) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	/*ThrowIfFailed*/(renderer->getDevice()->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&m_pipelineState)));

}

DX12Technique::~DX12Technique() {

}

void DX12Technique::enable(Renderer* renderer) {
	// better to delegate the render state to the renderer, it can be
	// more clever about changes with current render state set.
	/*renderer->setRenderState(renderState);
	material->enable();*/

	DX12Renderer* dxRenderer = static_cast<DX12Renderer*>(renderer);

	dxRenderer->getCmdList()->Reset(dxRenderer->getCmdAllocator(), m_pipelineState.Get());
}

