#pragma once

/* orders of class functions */
/*---------------------------------------*/
class SampleClass {
public:
#pragma region C/Dtor
#pragma endregion

#pragma region Getter
#pragma endregion

#pragma region Setter
#pragma endregion
};
/*---------------------------------------*/

/* orders of header */
/*---------------------------------------*/
#pragma region Pragma
#pragma endregion


#pragma region Include
#pragma endregion


#pragma region Define
#pragma endregion


#pragma region Using
#pragma endregion


#pragma region ClassForwardDecl
#pragma endregion


#pragma region EnumClass
#pragma endregion


#pragma region Variable
#pragma endregion


#pragma region Struct
#pragma endregion


#pragma region Function
#pragma endregion


#pragma region NameSpace
#pragma endregion


#pragma region Class
#pragma endregion
/*---------------------------------------*/
#pragma region Pragma
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma endregion


#pragma region Include
/* Windows */
#include <windows.h>
#include <Mmsystem.h>

/* C */
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <crtdbg.h>
#include <comdef.h>
#include <wrl.h>
#include <shellapi.h>

/* C++ */
#include <memory>
#include <string>
#include <fstream>
#include <random>
#include <functional>
#include <filesystem>
#include <type_traits>

/* STL Containers */
#include <span>
#include <array>
#include <vector>
#include <forward_list>
#include <deque>
#include <map>
#include <set>
#include <bitset>
#include <unordered_set>
#include <unordered_map>
#include <queue>

/* DirectX */
#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <D3d12SDKLayers.h>
#include <d3dx12.h>
#include "SimpleMath.h"

/* Custom */
#include "Common.h"
#pragma endregion


#pragma region Define
#define WIN32_LEAN_AND_MEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

/* ifdef */
#ifdef _DEBUG

// �޸� ���� �˻�, new�� �ϰ� delete�� ���� ���� ��� �� ��ġ�� �����ش�.
#define _CRTDBG_MAP_ALLOC

#ifdef _DEBUG
#ifdef UNICODE                                                                                      
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else                                                                                                    
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")   
#endif                                                                                                   
#endif


#else

#define new new

#endif