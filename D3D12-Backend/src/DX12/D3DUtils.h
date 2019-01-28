#pragma once

#include "DX12.h"

namespace D3DUtils {
	static wComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device, 
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		wComPtr<ID3D12Resource>& uploadBuffer);


};
