#pragma once

#include "DX12.h"

namespace D3DUtils {
	wComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device, 
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ID3D12Resource* uploadBuffer);

	void UpdateDefaultBufferData(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* data,
		UINT64 byteSize,
		UINT64 offset,
		ID3D12Resource* defaultBuffer,
		ID3D12Resource* uploadBuffer
	);

};

