#pragma once

#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"

#define ThrowIfFailed(hr) { \
	if (FAILED(hr)) { \
		throw std::exception(); \
	} \
}

template <typename T>
using wComPtr = Microsoft::WRL::ComPtr<T>;