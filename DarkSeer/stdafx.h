#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <assert.h>
#include <limits>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <memory>
#include <random>

//directx12
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
// D3D12 extension library.
#include "3rdParty/d3dx12.h"
// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

#undef min
#undef max
#undef CreateWindow

#include "Utility/Integrals.h"
#include "Utility/RawInputEnums.h"