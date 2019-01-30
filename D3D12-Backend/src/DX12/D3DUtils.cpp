#include "D3DUtils.h"

#include "DX12.h"

wComPtr<ID3D12Resource> D3DUtils::CreateDefaultBuffer(
	ID3D12Device * device, 
	ID3D12GraphicsCommandList * cmdList, 
	const void * initData, 
	UINT64 byteSize, 
	ID3D12Resource* uploadBuffer)
{
	wComPtr<ID3D12Resource> defaultBuffer;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// Creating an intermediate upload buffer to copy CPU memory data into the default buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer)));

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Put in barriers in order to schedule for the data to be copied
	// to the default buffer resource.
	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST));
	// Helper function to copy CPU memory data into the intermediate
	//	heap buffer and from that copy the data into the default buffer
	UpdateSubresources<1>(
		cmdList, defaultBuffer.Get(), 
		uploadBuffer, 0, 0, 
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

void D3DUtils::UpdateDefaultBufferData(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* data,
	UINT64 byteSize,
	UINT64 offset,
	ID3D12Resource* defaultBuffer,
	ID3D12Resource** uploadBuffer) 
{
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer)));
	(*uploadBuffer)->SetName(L"Vertex upload heap");

	// Put in barriers in order to schedule for the data to be copied
	// to the default buffer resource.
	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_COPY_DEST));

	//ThrowIfFailed(uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&data)));
	//void* dataBegin = nullptr;
	//D3D12_RANGE range = { 0, 0 }; //We do not intend to read this resource on the CPU.
	//(*uploadBuffer)->Map(0, &range, &dataBegin);
	//memcpy(dataBegin, data, byteSize);
	//(*uploadBuffer)->Unmap(0, nullptr);
	BYTE* pData;
	(*uploadBuffer)->Map(0, NULL, reinterpret_cast<void**>(&pData));
	memcpy(pData, data, byteSize);
	(*uploadBuffer)->Unmap(0, NULL);

	// Helper function to copy CPU memory data into the intermediate
	//	heap buffer and from that copy the data into the default buffer
	cmdList->CopyBufferRegion(defaultBuffer, offset, *uploadBuffer, 0, byteSize);
	
	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ));
}
