#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <assert.h>
#include <intrin.h>
#include <stdint.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>
#include <functional>
#include <immintrin.h>

// directx12
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
// D3D12 extension library.
#include <d3dx12.h>
// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

#include <MemoryLeakDetection.h>

#include <TaggedPrimitives.h>

#include <DSMath.h>
#include <DirectXHelpers.h>
#include <EntityAdmin.h>
#include <MemoryDefines.h>
#include <RawInput.Enums.h>
#include <MM256FlagLUT.h>