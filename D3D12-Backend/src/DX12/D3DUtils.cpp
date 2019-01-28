#include "D3DUtils.h"

#include "DX12.h"

wComPtr<ID3D12Resource> D3DUtils::CreateDefaultBuffer(
	ID3D12Device * device, 
	ID3D12GraphicsCommandList * cmdList, 
	const void * initData, 
	UINT64 byteSize, 
	wComPtr<ID3D12Resource>& uploadBuffer)
{
	wComPtr<ID3D12Resource> defaultBuffer;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// Creatin an intermediate upload buffer to copy CPU memory data into the default buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Put in barriers in order to schedule for the data to be copied
	// to the default buffer resource.
	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST));
	// Helper function to copy CPU memory data into the intermediate
	//	heap buffer and from that copy the data into the default buffer
	UpdateSubresources<1>(
		cmdList, defaultBuffer.Get(), 
		uploadBuffer.Get(), 0, 0, 
		1, &subResourceData);
	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, 
			D3D12_RESOURCE_STATE_GENERIC_READ));

	// The uploadBuffer can not be released as the above function calls has not 
	//  been executed yet.
	return defaultBuffer;
}