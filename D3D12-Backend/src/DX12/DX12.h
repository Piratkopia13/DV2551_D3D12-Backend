#pragma once

#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"

template <typename T>
using wComPtr = Microsoft::WRL::ComPtr<T>;