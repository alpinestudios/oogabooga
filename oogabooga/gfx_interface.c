#if GFX_RENDERER == GFX_RENDERER_D3D11
	#include <d3d11.h>
	#include <dxgi.h>
	#include <dxgi1_2.h>
	#include <d3dcompiler.h>
	#include <dxgidebug.h>
	#include <d3dcommon.h>
	typedef ID3D11ShaderResourceView * Gfx_Handle;
	
#elif GFX_RENDERER == GFX_RENDERER_VULKAN
	#error "We only have a D3D11 renderer at the moment"
#elif GFX_RENDERER == GFX_RENDERER_METAL
	#error "We only have a D3D11 renderer at the moment"
#else
	#error "Unknown renderer GFX_RENDERER defined"
#endif


typedef struct Gfx_State {

	// config
	bool enable_vsync;

	// readonly
	bool _can_vsync;
	
} Gfx_State;
Gfx_State gfx;

forward_global const Gfx_Handle GFX_INVALID_HANDLE;

typedef struct Gfx_Image {
	u32 width, height;
	u8 *data;
	Gfx_Handle gfx_handle;
	Allocator allocator;
} Gfx_Image;

