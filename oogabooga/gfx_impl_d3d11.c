

#if !OOGABOOGA_DEV

#include "d3d11_image_shader_bytecode.c"

#endif

#define D3D11Release(x) x->lpVtbl->Release(x)
#define VTABLE(proc, ...) FIRST_ARG(__VA_ARGS__)->lpVtbl->proc(__VA_ARGS__)

const Gfx_Handle GFX_INVALID_HANDLE = 0;

string temp_win32_null_terminated_wide_to_fixed_utf8(const u16 *utf16);

typedef struct D3D11_Vertex {
	Vector4 position;
	Vector2 uv;
	Vector4 color;
	int texture_index;
} D3D11_Vertex;

ID3D11Debug *d3d11_debug = 0;

ID3D11Device* d3d11_device = 0;
ID3D11DeviceContext* d3d11_context = 0;
ID3D11RenderTargetView *d3d11_window_render_target_view = 0;
ID3D11Texture2D *d3d11_back_buffer = 0;
D3D_DRIVER_TYPE d3d11_driver_type = 0;
D3D_FEATURE_LEVEL d3d11_feature_level = 0;

IDXGISwapChain1 *d3d11_swap_chain = 0;
DXGI_SWAP_CHAIN_DESC1 d3d11_swap_chain_desc = ZERO(DXGI_SWAP_CHAIN_DESC1);
u32 d3d11_swap_chain_width = 0;
u32 d3d11_swap_chain_height = 0;

ID3D11BlendState *d3d11_blend_state = 0;
ID3D11RasterizerState *d3d11_rasterizer = 0;
ID3D11SamplerState *d3d11_image_sampler = 0;

ID3D11VertexShader *d3d11_image_vertex_shader = 0;
ID3D11PixelShader  *d3d11_image_pixel_shader = 0;
ID3D11InputLayout  *d3d11_image_vertex_layout = 0;

ID3D11Buffer *d3d11_quad_vbo = 0;
u32 d3d11_quad_vbo_size = 0;

#define win32_check_hr(hr) win32_check_hr_impl(hr, __LINE__, __FILE__);
void win32_check_hr_impl(HRESULT hr, u32 line, const char* file_name) {
    if (FAILED(hr)) {
        LPVOID errorMsg;
        DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                        FORMAT_MESSAGE_FROM_SYSTEM | 
                        FORMAT_MESSAGE_IGNORE_INSERTS;

        DWORD messageLength = FormatMessage(
            dwFlags,
            NULL,
            hr,
            MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
            (LPTSTR) &errorMsg,
            0,
            NULL );

        if (messageLength > 0) {
            MessageBox(NULL, (LPCTSTR)errorMsg, TEXT("Error"), MB_OK | MB_ICONERROR);
        } else {
            MessageBox(NULL, TEXT("Failed to retrieve error message."), TEXT("Error"), MB_OK | MB_ICONERROR);
        }

        panic("win32 hr failed in file %cs on line %d", file_name, line);
    }
}

void d3d11_update_swapchain() {

	HRESULT hr;
	bool create = !d3d11_swap_chain;

	if (create) {
		DXGI_SWAP_CHAIN_DESC1 scd = ZERO(DXGI_SWAP_CHAIN_DESC1);
		scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//scd.BufferDesc.RefreshRate.Numerator = xx st.refresh_rate;
		//scd.BufferDesc.RefreshRate.Denominator = 1;
		
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.SampleDesc.Count = 1; 
		scd.SampleDesc.Quality = 0;
		if (d3d11_feature_level < D3D_FEATURE_LEVEL_11_0) {
			scd.Scaling = DXGI_SCALING_STRETCH; // for compatability with 7
		}
		
		// Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
		// for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
		// for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
		if (d3d11_feature_level >= D3D_FEATURE_LEVEL_11_0) {
			// this is supported only on FLIP presentation model
			scd.Scaling = DXGI_SCALING_NONE;
			scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			scd.BufferCount = 3;
			gfx._can_vsync = false;
			log_verbose("Present mode is flip discard, 3 buffers");
		} else {
			scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			scd.BufferCount = 2;
			gfx._can_vsync = true;
			log_verbose("Present mode is discard, 2 buffers");
		}
		
		// Obtain DXGI factory from device
		IDXGIDevice *dxgi_device;
		hr = VTABLE(QueryInterface, d3d11_device, &IID_IDXGIDevice, cast(void**)&dxgi_device);
		win32_check_hr(hr);
		
		IDXGIAdapter *adapter;
		hr = VTABLE(GetAdapter, dxgi_device, &adapter);
		win32_check_hr(hr);
		
		IDXGIFactory2 *dxgi_factory;
		hr = VTABLE(GetParent, adapter, &IID_IDXGIFactory2, cast(void**)&dxgi_factory); 
		win32_check_hr(hr);
	
		hr = VTABLE(CreateSwapChainForHwnd, dxgi_factory, (IUnknown*)d3d11_device, window._os_handle, &scd, 0, 0, &d3d11_swap_chain); 	
		win32_check_hr(hr);
		
		RECT client_rect;
		bool ok = GetClientRect(window._os_handle, &client_rect);
		assert(ok, "GetClientRect failed with error code %lu", GetLastError());
		
		d3d11_swap_chain_width  = client_rect.right-client_rect.left;
		d3d11_swap_chain_height = client_rect.bottom-client_rect.top;
		
		// store the swap chain description, as created by CreateSwapChainForHwnd
		hr = VTABLE(GetDesc1, d3d11_swap_chain, &d3d11_swap_chain_desc);
		win32_check_hr(hr);
		
		// disable alt enter
		VTABLE(MakeWindowAssociation, dxgi_factory, window._os_handle, cast (u32) DXGI_MWA_NO_ALT_ENTER); 
		
		D3D11Release(dxgi_device);
		D3D11Release(adapter);
		D3D11Release(dxgi_factory);
		
		log("Created swap chain of size %dx%d", d3d11_swap_chain_width, d3d11_swap_chain_height);
	} else {
		if (d3d11_window_render_target_view) D3D11Release(d3d11_window_render_target_view);
		if (d3d11_back_buffer) D3D11Release(d3d11_back_buffer);
		
		RECT client_rect;
		bool ok = GetClientRect(window._os_handle, &client_rect);
		assert(ok, "GetClientRect failed with error code %lu", GetLastError());
		
		u32 window_width  = client_rect.right-client_rect.left;
		u32 window_height = client_rect.bottom-client_rect.top;
		
		hr = VTABLE(ResizeBuffers, d3d11_swap_chain, d3d11_swap_chain_desc.BufferCount, window_width, window_height, d3d11_swap_chain_desc.Format, d3d11_swap_chain_desc.Flags);
		win32_check_hr(hr);
		
		// update swap chain description
		hr = VTABLE(GetDesc1, d3d11_swap_chain, &d3d11_swap_chain_desc);
		win32_check_hr(hr);
		
		log("Resized swap chain from %dx%d to %dx%d", d3d11_swap_chain_width, d3d11_swap_chain_height, window_width, window_height);
		
		d3d11_swap_chain_width  = window_width;
		d3d11_swap_chain_height = window_height;
	}
	
	
	
	
	hr = VTABLE(GetBuffer, d3d11_swap_chain, 0, &IID_ID3D11Texture2D, (void**)&d3d11_back_buffer);
	win32_check_hr(hr);
	hr = VTABLE(CreateRenderTargetView, d3d11_device, (ID3D11Resource*)d3d11_back_buffer, 0, &d3d11_window_render_target_view); 
	win32_check_hr(hr);
}

void gfx_init() {

	gfx.enable_vsync = false;

	log_verbose("d3d11 gfx_init");

    HWND hwnd = window._os_handle;
	HRESULT hr = S_OK;

	D3D11_CREATE_DEVICE_FLAG flags = 0; 
#if CONFIGURATION == DEBUG 
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driver_types[] = {
		D3D_DRIVER_TYPE_HARDWARE, 
		D3D_DRIVER_TYPE_WARP, 
		D3D_DRIVER_TYPE_REFERENCE
	};
	s64 num_drivers = sizeof(driver_types)/sizeof(D3D_DRIVER_TYPE);
	
	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1, 
		D3D_FEATURE_LEVEL_11_0, 
		D3D_FEATURE_LEVEL_10_1, 
		D3D_FEATURE_LEVEL_10_0
	};
	s64 num_feature_levels = sizeof(feature_levels)/sizeof(D3D_FEATURE_LEVEL);
	
	for (s64 i = 0; i < num_drivers; i++) {
		d3d11_driver_type = driver_types[i]; 
		
		hr = D3D11CreateDevice(0, d3d11_driver_type, 0, flags, feature_levels, num_feature_levels, D3D11_SDK_VERSION, &d3d11_device, &d3d11_feature_level, &d3d11_context);
		
		if (hr == E_INVALIDARG) {
			// 11_1 not recognized in 11.0
			hr = D3D11CreateDevice(0, d3d11_driver_type, 0, flags, feature_levels+1, num_feature_levels-1, D3D11_SDK_VERSION, &d3d11_device, &d3d11_feature_level, &d3d11_context);
		}
		
		if (SUCCEEDED(hr)) break;
		
		log_verbose("Failed driver type number %d (%d)", i, driver_types[i]);
	}
	
	win32_check_hr(hr);
	assert(d3d11_device != 0, "D3D11CreateDevice failed");

#if CONFIGURATION == DEBUG
	hr = VTABLE(QueryInterface, d3d11_device, &IID_ID3D11Debug, (void**)&d3d11_debug);
	if (SUCCEEDED(hr)) {
		log_verbose("D3D11 debug is active");
	}
#endif
	
	log_verbose("Created D3D11 device");
	
	IDXGIDevice *dxgi_device = 0;
	IDXGIAdapter *target_adapter = 0;
	hr = VTABLE(QueryInterface, d3d11_device, &IID_IDXGIDevice, (void **)&dxgi_device);
	
	
	hr = VTABLE(GetAdapter, dxgi_device, &target_adapter);
    if (SUCCEEDED(hr)) {
        DXGI_ADAPTER_DESC adapter_desc = ZERO(DXGI_ADAPTER_DESC);
        hr = VTABLE(GetDesc, target_adapter, &adapter_desc);
        if (SUCCEEDED(hr)) {
            string desc = temp_win32_null_terminated_wide_to_fixed_utf8(adapter_desc.Description);
            log("D3D11 adapter is: %s", desc);
        } else {
            log_error("Failed to get adapter description");
        }
    } else {
        log_error("Failed querying targeted d3d11_ adapter");
    }
    
    d3d11_update_swapchain();
    
    {
	    D3D11_BLEND_DESC bd = ZERO(D3D11_BLEND_DESC);
	    bd.RenderTarget[0].BlendEnable = TRUE;
	    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	    hr = VTABLE(CreateBlendState, d3d11_device, &bd, &d3d11_blend_state);
	    win32_check_hr(hr);
	    VTABLE(OMSetBlendState, d3d11_context, d3d11_blend_state, NULL, 0xffffffff);
	}
	
	{
	    D3D11_RASTERIZER_DESC desc = ZERO(D3D11_RASTERIZER_DESC);
	    desc.FillMode = D3D11_FILL_SOLID;
	    //desc.CullMode = D3D11_CULL_BACK;
	    desc.FrontCounterClockwise = FALSE;
	    desc.DepthClipEnable = FALSE;
	    desc.CullMode = D3D11_CULL_NONE;
	    hr = VTABLE(CreateRasterizerState, d3d11_device, &desc, &d3d11_rasterizer);
	    win32_check_hr(hr);
	    VTABLE(RSSetState, d3d11_context, d3d11_rasterizer);
	}
	
	// COnst buffer
	/*{
	    D3D11_BUFFER_DESC bd;
	    bd.ByteWidth = align_forward(sizeof(GlobalConstBuffer), 16);
	    bd.Usage = D3D11_USAGE_DYNAMIC;
	    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	    ID3D11Device_CreateBuffer(dx_state.d3d_device, &bd, NULL, &dx_state.const_buffer_resource);
	}*/
	
	/*{
	    D3D11_BUFFER_DESC bd;
	    bd.ByteWidth = align_forward(sizeof(BatchUniforms), 16);
	    bd.Usage = D3D11_USAGE_DYNAMIC;
	    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	    ID3D11Device_CreateBuffer(dx_state.d3d_device, &bd, NULL, &render_st.batch.ubo);
	}*/
	
	{
	    D3D11_SAMPLER_DESC sd = ZERO(D3D11_SAMPLER_DESC);
	    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	    hr = VTABLE(CreateSamplerState, d3d11_device, &sd, &d3d11_image_sampler);
	    win32_check_hr(hr);
	}
	
	// We are ooga booga devs so we read the file and compile
#if OOGABOOGA_DEV
	
	string source;
	bool source_ok = os_read_entire_file(fxstr("oogabooga/dev/d3d11_image_shader.hlsl"), &source, get_heap_allocator()); // #Leak
	assert(source_ok, "Could not open d3d11_image_shader source");
	
	// Compile vertex shader
    ID3DBlob* vs_blob = NULL;
    ID3DBlob* err_blob = NULL;
    hr = D3DCompile((char*)source.data, source.count, 0, 0, 0, "vs_main", "vs_5_0", 0, 0, &vs_blob, &err_blob);
	assert(SUCCEEDED(hr), "Vertex Shader Compilation Error: %cs\n", (char*)VTABLE(GetBufferPointer, err_blob));

    // Compile pixel shader
    ID3DBlob* ps_blob = NULL;
    hr = D3DCompile((char*)source.data, source.count, 0, 0, 0, "ps_main", "ps_5_0", 0, 0, &ps_blob, &err_blob);
    assert(SUCCEEDED(hr), "Vertex Shader Compilation Error: %cs\n", (char*)VTABLE(GetBufferPointer, err_blob));

	void *vs_buffer = VTABLE(GetBufferPointer, vs_blob);
	u64   vs_size   = VTABLE(GetBufferSize, vs_blob);
	void *ps_buffer = VTABLE(GetBufferPointer, ps_blob);
	u64   ps_size   = VTABLE(GetBufferSize, ps_blob);

    log_verbose("Shaders compiled");
    
    ///
    // Dump blobs to the .c
    File blob_file = os_file_open(fxstr("oogabooga/d3d11_image_shader_bytecode.c"), O_WRITE | O_CREATE);
    os_file_write_string(blob_file, fxstr("/*\n"));
    os_file_write_string(blob_file, fxstr("<<<<<< Bytecode compiled from HLSL code below: >>>>>>\n\n"));
    os_file_write_string(blob_file, source);
    os_file_write_string(blob_file, fxstr("\n*/\n\n"));
    
    os_file_write_string(blob_file, fxstr("const u8 IMAGE_SHADER_VERTEX_BLOB_BYTES[] = {\n"));
    for (u64 i = 0; i < vs_size; i++) {
    	os_file_write_string(blob_file, tprint("0x%02x", (int)((u8*)vs_buffer)[i]));
    	if (i < vs_size-1) os_file_write_string(blob_file, fxstr(", "));
    	if (i % 15 == 0 && i != 0) os_file_write_string(blob_file, fxstr("\n"));
    }
    os_file_write_string(blob_file, fxstr("\n};\n"));
    
    os_file_write_string(blob_file, fxstr("const u8 IMAGE_SHADER_PIXEL_BLOB_BYTES[] = {\n"));
    for (u64 i = 0; i < ps_size; i++) {
    	os_file_write_string(blob_file, tprint("0x%02x", (int)((u8*)ps_buffer)[i]));
    	if (i < ps_size-1) os_file_write_string(blob_file, fxstr(", "));
    	if (i % 15 == 0 && i != 0) os_file_write_string(blob_file, fxstr("\n"));
    }
    os_file_write_string(blob_file, fxstr("\n};\n"));
    os_file_close(blob_file);
    
    
#else

	const void *vs_buffer = IMAGE_SHADER_VERTEX_BLOB_BYTES;
	u64 vs_size = sizeof(IMAGE_SHADER_VERTEX_BLOB_BYTES);
	const void *ps_buffer = IMAGE_SHADER_PIXEL_BLOB_BYTES;
	u64 ps_size = sizeof(IMAGE_SHADER_PIXEL_BLOB_BYTES);
	
    log_verbose("Cached shaders loaded");
#endif

    // Create the shaders
    hr = VTABLE(CreateVertexShader, d3d11_device, vs_buffer, vs_size, NULL, &d3d11_image_vertex_shader);
    win32_check_hr(hr);

    hr = VTABLE(CreatePixelShader, d3d11_device, ps_buffer, ps_size, NULL, &d3d11_image_pixel_shader);
    win32_check_hr(hr);

    log_verbose("Shaders created");

#if OOGABOOGA_DEV
	D3D11Release(vs_blob);
    D3D11Release(ps_blob);
#endif

	D3D11_INPUT_ELEMENT_DESC layout[4];
	memset(layout, 0, sizeof(layout));
	
	layout[0] = (D3D11_INPUT_ELEMENT_DESC){
		"POSITION", 0,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
		offsetof(D3D11_Vertex, position),
		D3D11_INPUT_PER_VERTEX_DATA, 0
	};
	layout[1] = (D3D11_INPUT_ELEMENT_DESC){
		"TEXCOORD", 0,
		DXGI_FORMAT_R32G32_FLOAT, 0,
		offsetof(D3D11_Vertex, uv),
		D3D11_INPUT_PER_VERTEX_DATA, 0
	};
	layout[2] = (D3D11_INPUT_ELEMENT_DESC){
		"COLOR", 0,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
		offsetof(D3D11_Vertex, color),
		D3D11_INPUT_PER_VERTEX_DATA, 0
	};
	layout[3] = (D3D11_INPUT_ELEMENT_DESC){
		"TEXTURE_INDEX", 0,
		DXGI_FORMAT_R32_SINT, 0,
		offsetof(D3D11_Vertex, texture_index),
		D3D11_INPUT_PER_VERTEX_DATA, 0
	};
	
	hr = VTABLE(CreateInputLayout, d3d11_device, layout, 4, vs_buffer, vs_size, &d3d11_image_vertex_layout);

	log_info("D3D11 init done");
}

void d3d11_draw_call(int number_of_rendered_quads, ID3D11ShaderResourceView **textures, u64 num_textures) {
	VTABLE(OMSetBlendState, d3d11_context, d3d11_blend_state, 0, 0xffffffff);
	VTABLE(OMSetRenderTargets, d3d11_context, 1, &d3d11_window_render_target_view, 0); 
	VTABLE(RSSetState, d3d11_context, d3d11_rasterizer);
	D3D11_VIEWPORT viewport = ZERO(D3D11_VIEWPORT);
	viewport.Width = d3d11_swap_chain_width;
	viewport.Height = d3d11_swap_chain_height;
	viewport.MaxDepth = 1.0;
	VTABLE(RSSetViewports, d3d11_context, 1, &viewport);
	
    UINT stride = sizeof(D3D11_Vertex);
    UINT offset = 0;
	
	VTABLE(IASetInputLayout, d3d11_context, d3d11_image_vertex_layout);
    VTABLE(IASetVertexBuffers, d3d11_context, 0, 1, &d3d11_quad_vbo, &stride, &offset);
    VTABLE(IASetPrimitiveTopology, d3d11_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    VTABLE(VSSetShader, d3d11_context, d3d11_image_vertex_shader, NULL, 0);
    VTABLE(PSSetShader, d3d11_context, d3d11_image_pixel_shader, NULL, 0);
    
    VTABLE(PSSetSamplers, d3d11_context, 0, 1, &d3d11_image_sampler);
    VTABLE(PSSetShaderResources, d3d11_context, 0, num_textures, textures);

    VTABLE(ClearRenderTargetView, d3d11_context, d3d11_window_render_target_view, (float*)&window.clear_color);

    VTABLE(Draw, d3d11_context, number_of_rendered_quads * 6, 0);
}

void gfx_update() {

	if (window.should_close) return;
	
	VTABLE(ClearRenderTargetView, d3d11_context, d3d11_window_render_target_view, (float*)&window.clear_color);

	f64 rest_before  = os_get_current_time_in_seconds();

	HRESULT hr;

	///
	// purge garbage
	for (u64 i = 0; i < draw_frame.garbage_stack_count; i++) {
		ID3D11ShaderResourceView *view = draw_frame.garbage_stack[i];
		ID3D11Resource *resource = 0;
		VTABLE(GetResource, view, &resource);
		
		ID3D11Texture2D *texture = 0;
		hr = VTABLE(QueryInterface, resource, &IID_ID3D11Texture2D, (void**)&texture);
		if (SUCCEEDED(hr)) {
			D3D11Release(view);
			D3D11Release(texture);
			log("Destroyed an image");
		} else {
			panic("Unhandled D3D11 resource deletion");
		}
	}

	///
	// Maybe resize swap chain
	RECT client_rect;
	bool ok = GetClientRect(window._os_handle, &client_rect);
	assert(ok, "GetClientRect failed with error code %lu", GetLastError());
	u32 window_width  = client_rect.right-client_rect.left;
	u32 window_height = client_rect.bottom-client_rect.top;
	if (window_width != d3d11_swap_chain_width || window_height != d3d11_swap_chain_height) {
		d3d11_update_swapchain();
	}

	///
	// Maybe grow quad vbo
	u32 required_size = sizeof(D3D11_Vertex) * draw_frame.num_blocks*QUADS_PER_BLOCK*6;
	if (required_size > d3d11_quad_vbo_size) {
		if (d3d11_quad_vbo) {
			D3D11Release(d3d11_quad_vbo);
		}
		D3D11_BUFFER_DESC desc = ZERO(D3D11_BUFFER_DESC);
		desc.Usage = D3D11_USAGE_DYNAMIC; 
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.ByteWidth = required_size;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		HRESULT hr = VTABLE(CreateBuffer, d3d11_device, &desc, 0, &d3d11_quad_vbo);
		assert(SUCCEEDED(hr), "CreateBuffer failed");
		d3d11_quad_vbo_size = required_size;
		
		log_verbose("Grew quad vbo to %d bytes.", d3d11_quad_vbo_size);
	}

	if (draw_frame.num_blocks > 0) {
		///
		// Render geometry from into vbo quad list
	    D3D11_MAPPED_SUBRESOURCE buffer_mapping;
		VTABLE(Map, d3d11_context, (ID3D11Resource*)d3d11_quad_vbo, 0, D3D11_MAP_WRITE_DISCARD, 0, &buffer_mapping);
		
		ID3D11ShaderResourceView *textures[32];
		ID3D11ShaderResourceView *last_texture = 0;
		u64 num_textures = 0;
		
		D3D11_Vertex* head = buffer_mapping.pData;
		D3D11_Vertex* pointer = head;
		u64 number_of_rendered_quads = 0;
		Draw_Quad_Block *block = &draw_frame.first_block;
		while (block != 0) {
			
			for (u64 i = 0; i < block->num_quads; i++) {
				Draw_Quad *q = &block->quad_buffer[i];
				
				int texture_index = -1;
				
				if (q->image) {
					if (!q->image->gfx_handle) {
						D3D11_TEXTURE2D_DESC desc = ZERO(D3D11_TEXTURE2D_DESC);
						desc.Width = q->image->width;
						desc.Height = q->image->height;
						desc.MipLevels = 1;
						desc.ArraySize = 1;
						desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						desc.SampleDesc.Count = 1;
						desc.SampleDesc.Quality = 0;
						desc.Usage = D3D11_USAGE_DEFAULT;
						desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
						desc.CPUAccessFlags = 0;
						desc.MiscFlags = 0;
						
						D3D11_SUBRESOURCE_DATA data = ZERO(D3D11_SUBRESOURCE_DATA);
						data.pSysMem = q->image->data;
						data.SysMemPitch  = q->image->width * 4; // #Magicvalue assuming 4 channels
						
						ID3D11Texture2D* texture = 0;
						HRESULT hr = VTABLE(CreateTexture2D, d3d11_device, &desc, &data, &texture);
						win32_check_hr(hr);
						
						hr = VTABLE(CreateShaderResourceView, d3d11_device, (ID3D11Resource*)texture, 0, &q->image->gfx_handle);
						win32_check_hr(hr);
						
						log_verbose("Created an image of width %d and height %d.", q->image->width, q->image->height);
					}
					
					if (last_texture == q->image->gfx_handle) {
						texture_index = (int)(num_textures-1);
					} else {
						// First look if texture is already bound
						for (u64 j = 0; j < num_textures; j++) {
							if (textures[j] == q->image->gfx_handle) {
								texture_index = (int)j;
								break;
							}
						}
						// Otherwise use a new slot
						if (texture_index <= -1) {
							if (num_textures >= 32) {
								// If max textures reached, make a draw call and start over
								d3d11_draw_call(number_of_rendered_quads, textures, num_textures);
								num_textures = 0;
								texture_index = 0;
								number_of_rendered_quads = 0;
								pointer = head;
							} else {
								texture_index = (int)num_textures;
								num_textures += 1;
							}
						}
					}
					textures[texture_index] = q->image->gfx_handle;
					last_texture = q->image->gfx_handle;
				}
				
				// We will write to 6 vertices for the one quad (two tris)
				D3D11_Vertex* BL  = pointer + 0;
				D3D11_Vertex* TL  = pointer + 1;
				D3D11_Vertex* TR  = pointer + 2;
				D3D11_Vertex* BL2 = pointer + 3;
				D3D11_Vertex* TR2 = pointer + 4;
				D3D11_Vertex* BR  = pointer + 5;
				pointer += 6;
				
				BL->position = v4(q->bottom_left.x,  q->bottom_left.y,  0, 1);
				TL->position = v4(q->top_left.x,     q->top_left.y,     0, 1);
				TR->position = v4(q->top_right.x,    q->top_right.y,    0, 1);
				BR->position = v4(q->bottom_right.x, q->bottom_right.y, 0, 1);
				
				BL->uv = v2(q->uv.x1, q->uv.y1);
				TL->uv = v2(q->uv.x1, q->uv.y2);
				TR->uv = v2(q->uv.x2, q->uv.y2);
				BR->uv = v2(q->uv.x2, q->uv.y1);
				
				BL->color = TL->color = TR->color = BR->color = q->color;
				
				BL->texture_index=TL->texture_index=TR->texture_index=BR->texture_index = texture_index;
				
				*BL2 = *BL;
				*TR2 = *TR;
				
				number_of_rendered_quads += 1;
			}
			
			block = block->next;
		}
	    
		VTABLE(Unmap, d3d11_context, (ID3D11Resource*)d3d11_quad_vbo, 0);
		
		///
		// Draw call
		
		d3d11_draw_call(number_of_rendered_quads, textures, num_textures);
    }
    
    f64 rest_after  = os_get_current_time_in_seconds();
    if (is_key_just_pressed('E')) 
    	log("The rest took %.2fms", (rest_after-rest_before)*1000.0);
    
    f64 before_present = os_get_current_time_in_seconds();
    hr = VTABLE(Present, d3d11_swap_chain, gfx._can_vsync && gfx.enable_vsync, 0);
    f64 after = os_get_current_time_in_seconds();
    if (is_key_just_pressed('E')) 
    	log("Present took %.2fms", (after-before_present)*1000.0);
	win32_check_hr(hr);
	
	reset_draw_frame(&draw_frame);
}
