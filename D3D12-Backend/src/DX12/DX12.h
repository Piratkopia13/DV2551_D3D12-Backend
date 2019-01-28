#pragma once

#include <wrl.h>
#include <d3d12.h>

template <typename T>
using wComPtr = Microsoft::WRL::ComPtr<T>;