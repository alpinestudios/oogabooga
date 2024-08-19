


#define D3D11Release(x) x->lpVtbl->Release(x)

const Gfx_Handle GFX_INVALID_HANDLE = 0;

string temp_win32_null_terminated_wide_to_fixed_utf8(const u16 *utf16);

// We wanna pack this at some point
// #Cleanup #Memory why am I doing alignat(16)?
typedef struct alignat(16) D3D11_Vertex {
	
	Vector4 color;
	Vector4 position;
	Vector2 uv;
	Vector2 self_uv;
	s8 texture_index;
	u8 type;
	u8 sampler;
	u8 has_scissor;
	
	Vector4 userdata[VERTEX_2D_USER_DATA_COUNT];
	
	Vector4 scissor;
	
} D3D11_Vertex;

// #Global

ID3D11Debug *d3d11_debug = 0;

ID3D11Device *d3d11_device = 0;
ID3D11DeviceContext *d3d11_context = 0;
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
ID3D11SamplerState *d3d11_image_sampler_np_fp = 0;
ID3D11SamplerState *d3d11_image_sampler_nl_fl = 0;
ID3D11SamplerState *d3d11_image_sampler_np_fl = 0;
ID3D11SamplerState *d3d11_image_sampler_nl_fp = 0;

ID3D11VertexShader *d3d11_vertex_shader_for_2d = 0;
ID3D11PixelShader  *d3d11_fragment_shader_for_2d = 0;
ID3D11InputLayout  *d3d11_image_vertex_layout = 0;

ID3D11Buffer *d3d11_quad_vbo = 0;
u32 d3d11_quad_vbo_size = 0;
void *d3d11_staging_quad_buffer = 0;

ID3D11Buffer *d3d11_cbuffer = 0;
u64 d3d11_cbuffer_size = 0;

Draw_Quad *sort_quad_buffer = 0;
u64 sort_quad_buffer_size = 0;

const char* d3d11_stringify_category(D3D11_MESSAGE_CATEGORY category) {
    switch (category) {
    case D3D11_MESSAGE_CATEGORY_APPLICATION_DEFINED: return "Application Defined";
    case D3D11_MESSAGE_CATEGORY_MISCELLANEOUS: return "Miscellaneous";
    case D3D11_MESSAGE_CATEGORY_INITIALIZATION: return "Initialization";
    case D3D11_MESSAGE_CATEGORY_CLEANUP: return "Cleanup";
    case D3D11_MESSAGE_CATEGORY_COMPILATION: return "Compilation";
    case D3D11_MESSAGE_CATEGORY_STATE_CREATION: return "State Creation";
    case D3D11_MESSAGE_CATEGORY_STATE_SETTING: return "State Setting";
    case D3D11_MESSAGE_CATEGORY_STATE_GETTING: return "State Getting";
    case D3D11_MESSAGE_CATEGORY_RESOURCE_MANIPULATION: return "Resource Manipulation";
    case D3D11_MESSAGE_CATEGORY_EXECUTION: return "Execution";
    default: return "Unknown";
    }
}

const char* d3d11_stringify_severity(D3D11_MESSAGE_SEVERITY severity) {
    switch (severity) {
    case D3D11_MESSAGE_SEVERITY_CORRUPTION: return "Corruption";
    case D3D11_MESSAGE_SEVERITY_ERROR: return "Error";
    case D3D11_MESSAGE_SEVERITY_WARNING: return "Warning";
    case D3D11_MESSAGE_SEVERITY_INFO: return "Info";
    case D3D11_MESSAGE_SEVERITY_MESSAGE: return "Message";
    default: return "Unknown";
    }
}
void CALLBACK d3d11_debug_callback(D3D11_MESSAGE_CATEGORY category, D3D11_MESSAGE_SEVERITY severity, D3D11_MESSAGE_ID id, const char* description)
{
	if (id == 391) {
		// Sigh:
		/*
			[WARNING]: D3D11 MESSAGE [Category: State Creation, Severity: Warning, id: 391]: ID3D11Device::CreateInputLayout: The provided input signature expects to read an element with SemanticName/Index: 'SAMPLER_INDEX'/0 and component(s) of the type 'uint32'.  However, the matching entry in the Input Layout declaration, element[5], specifies mismatched format: 'R8_SINT'.  This is not an error, since behavior is well defined: The element format determines what data conversion algorithm gets applied before it shows up in a shader register. Independently, the shader input signature defines how the shader will interpret the data that has been placed in its input registers, with no change in the bits stored.  It is valid for the application to reinterpret data as a different type once it is in the vertex shader, so this warning is issued just in case reinterpretation was not intended by the author.
		*/
		return;
	}

	string msg = tprint("D3D11 MESSAGE [Category: %cs, Severity: %cs, id: %d]: %cs", d3d11_stringify_category(category), d3d11_stringify_severity(severity), id, description);
	
	switch (severity) {
		case D3D11_MESSAGE_SEVERITY_CORRUPTION:
		case D3D11_MESSAGE_SEVERITY_ERROR:
			log_error(msg);
			break;
		case D3D11_MESSAGE_SEVERITY_WARNING:
			log_warning(msg);
			break;
		case D3D11_MESSAGE_SEVERITY_INFO:
			log_info(msg);
			break;
		case D3D11_MESSAGE_SEVERITY_MESSAGE:
			log_verbose(msg);
			break;
		default:
			log("Ligma");
			break;
	}
}
void
d3d11_output_debug_messages() {
	///
	// Check debug messages, output to stdout
	ID3D11InfoQueue* info_q = 0;
	HRESULT hr = ID3D11Device_QueryInterface(d3d11_device, &IID_ID3D11InfoQueue, (void**)&info_q);
	if (SUCCEEDED(hr)) {
		u64 msg_count = ID3D11InfoQueue_GetNumStoredMessagesAllowedByRetrievalFilter(info_q);
		for (u64 i = 0; i < msg_count; i++) {
		    SIZE_T msg_size = 0;
		    ID3D11InfoQueue_GetMessage(info_q, i, 0, &msg_size);
		
		    D3D11_MESSAGE* msg = (D3D11_MESSAGE*)talloc(msg_size);
		    if (msg) {
		        ID3D11InfoQueue_GetMessage(info_q, i, msg, &msg_size); // Get the actual message
		        
		        d3d11_debug_callback(msg->Category, msg->Severity, msg->ID, msg->pDescription);
		    }
		}
	}
}

#define d3d11_check_hr(hr) d3d11_check_hr_impl(hr, __LINE__, __FILE__);
void 
d3d11_check_hr_impl(HRESULT hr, u32 line, const char* file_name) {
	if (!SUCCEEDED(hr)) d3d11_output_debug_messages();
    win32_check_hr_impl(hr, line, file_name);
}

// Defined at the bottom of this file
// #Global
extern const char *d3d11_image_shader_source;



void d3d11_update_swapchain() {

	HRESULT hr;
	bool create = !d3d11_swap_chain;

	if (create) {
		DXGI_SWAP_CHAIN_DESC1 scd = ZERO(DXGI_SWAP_CHAIN_DESC1);
		scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		
		//scd.BufferDesc.RefreshRate.Numerator = 0;
		//scd.BufferDesc.RefreshRate.Denominator = 1;
		
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.SampleDesc.Count = 1; 
		scd.SampleDesc.Quality = 0; // #Portability
		if (d3d11_feature_level < D3D_FEATURE_LEVEL_11_0) {
			scd.Scaling = DXGI_SCALING_STRETCH; // for compatability with 7
		}
		
		
		// Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
		// for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
		// for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
		if (d3d11_feature_level >= D3D_FEATURE_LEVEL_11_0) {
			// this is supported only on FLIP presentation model
			scd.Scaling = DXGI_SCALING_NONE;
			scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; 
			scd.BufferCount = 3;
			log_verbose("Present mode is flip discard, 3 buffers");
		} else {
			scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			scd.BufferCount = 2;
			log_verbose("Present mode is discard, 2 buffers");
		}
		
		
		// Obtain DXGI factory from device
		IDXGIDevice *dxgi_device = 0;
		hr = ID3D11Device_QueryInterface(d3d11_device, &IID_IDXGIDevice, cast(void**)&dxgi_device);
		d3d11_check_hr(hr);
		
		IDXGIAdapter *adapter;
		hr = IDXGIDevice_GetAdapter(dxgi_device, &adapter);
		d3d11_check_hr(hr);
		
		IDXGIFactory2 *dxgi_factory;
		hr = IDXGIAdapter_GetParent(adapter, &IID_IDXGIFactory2, cast(void**)&dxgi_factory); 
		d3d11_check_hr(hr);
	
		hr = IDXGIFactory2_CreateSwapChainForHwnd(dxgi_factory, (IUnknown*)d3d11_device, window._os_handle, &scd, 0, 0, &d3d11_swap_chain); 	
		d3d11_check_hr(hr);
		
		RECT client_rect;
		bool ok = GetClientRect(window._os_handle, &client_rect);
		assert(ok, "GetClientRect failed with error code %lu", GetLastError());
		
		d3d11_swap_chain_width  = client_rect.right-client_rect.left;
		d3d11_swap_chain_height = client_rect.bottom-client_rect.top;
		
		// store the swap chain description, as created by CreateSwapChainForHwnd
		hr = IDXGISwapChain1_GetDesc1(d3d11_swap_chain, &d3d11_swap_chain_desc);
		d3d11_check_hr(hr);
		
		// disable alt enter
		IDXGIFactory_MakeWindowAssociation(dxgi_factory, window._os_handle, cast (u32) DXGI_MWA_NO_ALT_ENTER); 
		
		IDXGIDevice_Release(dxgi_device);
		IDXGIAdapter_Release(adapter);
		IDXGIFactory_Release(dxgi_factory);
		
		log("Created swap chain of size %dx%d", d3d11_swap_chain_width, d3d11_swap_chain_height);
	} else {
		if (d3d11_window_render_target_view) D3D11Release(d3d11_window_render_target_view);
		if (d3d11_back_buffer) D3D11Release(d3d11_back_buffer);
		
		RECT client_rect;
		bool ok = GetClientRect(window._os_handle, &client_rect);
		assert(ok, "GetClientRect failed with error code %lu", GetLastError());
		
		u32 window_width  = client_rect.right-client_rect.left;
		u32 window_height = client_rect.bottom-client_rect.top;
		
		hr = IDXGISwapChain1_ResizeBuffers(d3d11_swap_chain, d3d11_swap_chain_desc.BufferCount, window_width, window_height, d3d11_swap_chain_desc.Format, d3d11_swap_chain_desc.Flags);
		d3d11_check_hr(hr);
		
		// update swap chain description
		hr = IDXGISwapChain1_GetDesc1(d3d11_swap_chain, &d3d11_swap_chain_desc);
		d3d11_check_hr(hr);
		
		log("Resized swap chain from %dx%d to %dx%d", d3d11_swap_chain_width, d3d11_swap_chain_height, window_width, window_height);
		
		d3d11_swap_chain_width  = window_width;
		d3d11_swap_chain_height = window_height;
	}
	
	
	
	
	hr = IDXGISwapChain1_GetBuffer(d3d11_swap_chain, 0, &IID_ID3D11Texture2D, (void**)&d3d11_back_buffer);
	d3d11_check_hr(hr);
	hr = ID3D11Device_CreateRenderTargetView(d3d11_device, (ID3D11Resource*)d3d11_back_buffer, 0, &d3d11_window_render_target_view); 
	d3d11_check_hr(hr);
}

bool
d3d11_compile_shader(string source) {

	source = string_replace_all(source, STR("$INJECT_PIXEL_POST_PROCESS"), STR("float4 pixel_shader_extension(PS_INPUT input, float4 color) { return color; }"), get_temporary_allocator());
	source = string_replace_all(source, STR("$VERTEX_2D_USER_DATA_COUNT"), tprint("%d", VERTEX_2D_USER_DATA_COUNT), get_temporary_allocator());
	
	// #Leak on recompile
	
	///
	// Make default shaders
	
	// Compile vertex shader
    ID3DBlob* vs_blob = NULL;
    ID3DBlob* err_blob = NULL;
    HRESULT hr = D3DCompile((char*)source.data, source.count, 0, 0, 0, "vs_main", "vs_5_0", 0, 0, &vs_blob, &err_blob);
	if (!SUCCEEDED(hr)) {
		log_error("Vertex Shader Compilation Error: %cs\n", (char*)ID3D10Blob_GetBufferPointer(err_blob));
		return false;
	}

    // Compile pixel shader
    ID3DBlob* ps_blob = NULL;
    hr = D3DCompile((char*)source.data, source.count, 0, 0, 0, "ps_main", "ps_5_0", 0, 0, &ps_blob, &err_blob);
	if (!SUCCEEDED(hr)) {
		log_error("Fragment Shader Compilation Error: %cs\n", (char*)ID3D10Blob_GetBufferPointer(err_blob));
		return false;
	}

	void *vs_buffer = ID3D10Blob_GetBufferPointer(vs_blob);
	u64   vs_size   = ID3D10Blob_GetBufferSize(vs_blob);
	void *ps_buffer = ID3D10Blob_GetBufferPointer(ps_blob);
	u64   ps_size   = ID3D10Blob_GetBufferSize(ps_blob);

    log_verbose("Shaders compiled");
    
    
    // Create the shaders
    hr = ID3D11Device_CreateVertexShader(d3d11_device, vs_buffer, vs_size, NULL, &d3d11_vertex_shader_for_2d);
    d3d11_check_hr(hr);

    hr = ID3D11Device_CreatePixelShader(d3d11_device, ps_buffer, ps_size, NULL, &d3d11_fragment_shader_for_2d);
    d3d11_check_hr(hr);

    log_verbose("Shaders created");



	#define layout_base_count 9
	D3D11_INPUT_ELEMENT_DESC layout[layout_base_count+VERTEX_2D_USER_DATA_COUNT];
	memset(layout, 0, sizeof(layout));
	
	layout[0].SemanticName = "POSITION";
	layout[0].SemanticIndex = 0;
	layout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	layout[0].InputSlot = 0;
	layout[0].AlignedByteOffset = offsetof(D3D11_Vertex, position);
	layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[0].InstanceDataStepRate = 0;
	
	layout[1].SemanticName = "TEXCOORD";
	layout[1].SemanticIndex = 0;
	layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	layout[1].InputSlot = 0;
	layout[1].AlignedByteOffset = offsetof(D3D11_Vertex, uv);
	layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[1].InstanceDataStepRate = 0;
	
	layout[2].SemanticName = "COLOR";
	layout[2].SemanticIndex = 0;
	layout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	layout[2].InputSlot = 0;
	layout[2].AlignedByteOffset = offsetof(D3D11_Vertex, color);
	layout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[2].InstanceDataStepRate = 0;
	
	layout[3].SemanticName = "TEXTURE_INDEX";
	layout[3].SemanticIndex = 0;
	layout[3].Format = DXGI_FORMAT_R8_SINT;
	layout[3].InputSlot = 0;
	layout[3].AlignedByteOffset = offsetof(D3D11_Vertex, texture_index);
	layout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[3].InstanceDataStepRate = 0;
	
	layout[4].SemanticName = "TYPE";
	layout[4].SemanticIndex = 0;
	layout[4].Format = DXGI_FORMAT_R8_UINT;
	layout[4].InputSlot = 0;
	layout[4].AlignedByteOffset = offsetof(D3D11_Vertex, type);
	layout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[4].InstanceDataStepRate = 0;
	
	layout[5].SemanticName = "SAMPLER_INDEX";
	layout[5].SemanticIndex = 0;
	layout[5].Format = DXGI_FORMAT_R8_SINT;
	layout[5].InputSlot = 0;
	layout[5].AlignedByteOffset = offsetof(D3D11_Vertex, sampler);
	layout[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[5].InstanceDataStepRate = 0;
	
	layout[6].SemanticName = "SELF_UV";
	layout[6].SemanticIndex = 0;
	layout[6].Format = DXGI_FORMAT_R32G32_FLOAT;
	layout[6].InputSlot = 0;
	layout[6].AlignedByteOffset = offsetof(D3D11_Vertex, self_uv);
	layout[6].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[6].InstanceDataStepRate = 0;
	
	layout[7].SemanticName = "SCISSOR";
	layout[7].SemanticIndex = 0;
	layout[7].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	layout[7].InputSlot = 0;
	layout[7].AlignedByteOffset = offsetof(D3D11_Vertex, scissor);
	layout[7].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[7].InstanceDataStepRate = 0;
	
	layout[8].SemanticName = "HAS_SCISSOR";
	layout[8].SemanticIndex = 0;
	layout[8].Format = DXGI_FORMAT_R8_UINT;
	layout[8].InputSlot = 0;
	layout[8].AlignedByteOffset = offsetof(D3D11_Vertex, has_scissor);
	layout[8].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout[8].InstanceDataStepRate = 0;
	
	for (int i = 0; i < VERTEX_2D_USER_DATA_COUNT; ++i) {
	    layout[layout_base_count + i].SemanticName = "USERDATA";
	    layout[layout_base_count + i].SemanticIndex = i;
	    layout[layout_base_count + i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	    layout[layout_base_count + i].InputSlot = 0;
	    layout[layout_base_count + i].AlignedByteOffset = offsetof(D3D11_Vertex, userdata) + sizeof(Vector4) * i;
	    layout[layout_base_count + i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	}
	
	
	hr = ID3D11Device_CreateInputLayout(d3d11_device, layout, layout_base_count+VERTEX_2D_USER_DATA_COUNT, vs_buffer, vs_size, &d3d11_image_vertex_layout);
	d3d11_check_hr(hr);
	
	#undef layout_base_count

	D3D11Release(vs_blob);
    D3D11Release(ps_blob);

	return true;
}

void gfx_init() {

	window.enable_vsync = false;

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
	
	bool debug_failed = false;
	
	for (s64 i = 0; i < 2; i++) {
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
		if (SUCCEEDED(hr)) {
			break;
		} else {
			debug_failed = true;
			flags &= ~(D3D11_CREATE_DEVICE_DEBUG);
			
		}
	}
	
	d3d11_check_hr(hr);
	
	if (debug_failed) {
		log_error("We could not init D3D11 with DEBUG flag. To fix this, you can try:\n1. Go to windows settings\n2. Go to System -> Optional features\n3. Add the feature called \"Graphics Tools\"\n4. Restart your computer\n5. Be frustrated that windows is like this.\nhttps://devblogs.microsoft.com/cppblog/visual-studio-2015-and-graphics-tools-for-windows-10/");
	}
	
	assert(d3d11_device != 0, "D3D11CreateDevice failed");

#if CONFIGURATION == DEBUG
	hr = ID3D11Device_QueryInterface(d3d11_device, &IID_ID3D11Debug, (void**)&d3d11_debug);
	if (SUCCEEDED(hr)) {
		log_verbose("D3D11 debug is active");
	}
#endif
	
	log_verbose("Created D3D11 device");
	
	IDXGIDevice *dxgi_device = 0;
	IDXGIAdapter *target_adapter = 0;
	hr = ID3D11Device_QueryInterface(d3d11_device, &IID_IDXGIDevice, (void **)&dxgi_device);
	
	
	hr = IDXGIDevice_GetAdapter(dxgi_device, &target_adapter);
    if (SUCCEEDED(hr)) {
        DXGI_ADAPTER_DESC adapter_desc = ZERO(DXGI_ADAPTER_DESC);
        hr = IDXGIAdapter_GetDesc(target_adapter, &adapter_desc);
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
	    hr = ID3D11Device_CreateBlendState(d3d11_device, &bd, &d3d11_blend_state);
	    d3d11_check_hr(hr);
	    ID3D11DeviceContext_OMSetBlendState(d3d11_context, d3d11_blend_state, NULL, 0xffffffff);
	}
	
	{
	    D3D11_RASTERIZER_DESC desc = ZERO(D3D11_RASTERIZER_DESC);
	    desc.FillMode = D3D11_FILL_SOLID;
	    //desc.CullMode = D3D11_CULL_BACK;
	    desc.FrontCounterClockwise = FALSE;
	    desc.DepthClipEnable = FALSE;
	    desc.CullMode = D3D11_CULL_NONE;
	    hr = ID3D11Device_CreateRasterizerState(d3d11_device, &desc, &d3d11_rasterizer);
	    d3d11_check_hr(hr);
	    ID3D11DeviceContext_RSSetState(d3d11_context, d3d11_rasterizer);
	}
	
	{
	    D3D11_SAMPLER_DESC sd = ZERO(D3D11_SAMPLER_DESC);
	    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	    
	    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	    hr = ID3D11Device_CreateSamplerState(d3d11_device, &sd, &d3d11_image_sampler_np_fp);
	    d3d11_check_hr(hr);
	    
	    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	    hr =ID3D11Device_CreateSamplerState(d3d11_device, &sd, &d3d11_image_sampler_nl_fl);
	    d3d11_check_hr(hr);
	    
	    sd.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	    hr = ID3D11Device_CreateSamplerState(d3d11_device, &sd, &d3d11_image_sampler_np_fl);
	    d3d11_check_hr(hr);
	    
	    sd.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	    hr = ID3D11Device_CreateSamplerState(d3d11_device, &sd, &d3d11_image_sampler_nl_fp);
	    d3d11_check_hr(hr);
	}
	
	string source = STR(d3d11_image_shader_source);
	
	bool ok = d3d11_compile_shader(source);
	
	assert(ok, "Failed compiling default shader");

	log_info("D3D11 init done");
	
}

void d3d11_draw_call(int number_of_rendered_quads, ID3D11ShaderResourceView **textures, u64 num_textures) {
	ID3D11DeviceContext_OMSetBlendState(d3d11_context, d3d11_blend_state, 0, 0xffffffff);
	ID3D11DeviceContext_OMSetRenderTargets(d3d11_context, 1, &d3d11_window_render_target_view, 0); 
	ID3D11DeviceContext_RSSetState(d3d11_context, d3d11_rasterizer);
	D3D11_VIEWPORT viewport = ZERO(D3D11_VIEWPORT);
	viewport.Width = d3d11_swap_chain_width;
	viewport.Height = d3d11_swap_chain_height;
	viewport.MaxDepth = 1.0;
	ID3D11DeviceContext_RSSetViewports(d3d11_context, 1, &viewport);
	
    UINT stride = sizeof(D3D11_Vertex);
    UINT offset = 0;
	
	ID3D11DeviceContext_IASetInputLayout(d3d11_context, d3d11_image_vertex_layout);
    ID3D11DeviceContext_IASetVertexBuffers(d3d11_context, 0, 1, &d3d11_quad_vbo, &stride, &offset);
    ID3D11DeviceContext_IASetPrimitiveTopology(d3d11_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11DeviceContext_VSSetShader(d3d11_context, d3d11_vertex_shader_for_2d, NULL, 0);
    ID3D11DeviceContext_PSSetShader(d3d11_context, d3d11_fragment_shader_for_2d, NULL, 0);
    
	if (draw_frame.cbuffer && d3d11_cbuffer && d3d11_cbuffer_size) {
		D3D11_MAPPED_SUBRESOURCE cbuffer_mapping;
		ID3D11DeviceContext_Map(
			d3d11_context, 
			(ID3D11Resource*)d3d11_cbuffer, 
			0, 
			D3D11_MAP_WRITE_DISCARD, 
			0, 
			&cbuffer_mapping
		);
		memcpy(cbuffer_mapping.pData, draw_frame.cbuffer, d3d11_cbuffer_size);
		ID3D11DeviceContext_Unmap(d3d11_context, (ID3D11Resource*)d3d11_cbuffer, 0);
		
		ID3D11DeviceContext_PSSetConstantBuffers(d3d11_context, 0, 1, &d3d11_cbuffer);
	}
    
    ID3D11DeviceContext_PSSetSamplers(d3d11_context, 0, 1, &d3d11_image_sampler_np_fp);
    ID3D11DeviceContext_PSSetSamplers(d3d11_context, 1, 1, &d3d11_image_sampler_nl_fl);
    ID3D11DeviceContext_PSSetSamplers(d3d11_context, 2, 1, &d3d11_image_sampler_np_fl);
    ID3D11DeviceContext_PSSetSamplers(d3d11_context, 3, 1, &d3d11_image_sampler_nl_fp);
    ID3D11DeviceContext_PSSetShaderResources(d3d11_context, 0, num_textures, textures);

    ID3D11DeviceContext_Draw(d3d11_context, number_of_rendered_quads * 6, 0);
}

void d3d11_process_draw_frame() {

	HRESULT hr;
	
	ID3D11DeviceContext_ClearRenderTargetView(d3d11_context, d3d11_window_render_target_view, (float*)&window.clear_color);
	
	if (!draw_frame.quad_buffer) return;

	u64 number_of_quads = growing_array_get_valid_count(draw_frame.quad_buffer);
	
	///
	// Maybe grow quad vbo
	u64 required_size = sizeof(D3D11_Vertex) * number_of_quads*6;

	if (required_size > d3d11_quad_vbo_size) {
		if (d3d11_quad_vbo) {
			D3D11Release(d3d11_quad_vbo);
			dealloc(get_heap_allocator(), d3d11_staging_quad_buffer);
		}
		u64 new_size = get_next_power_of_two(required_size);
		D3D11_BUFFER_DESC desc = ZERO(D3D11_BUFFER_DESC);
		desc.Usage = D3D11_USAGE_DYNAMIC; 
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.ByteWidth = new_size;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		HRESULT hr = ID3D11Device_CreateBuffer(d3d11_device, &desc, 0, &d3d11_quad_vbo);
		assert(SUCCEEDED(hr), "CreateBuffer failed");
		d3d11_quad_vbo_size = new_size;
		
		d3d11_staging_quad_buffer = alloc(get_heap_allocator(), d3d11_quad_vbo_size);
		assert((u64)d3d11_staging_quad_buffer%16 == 0);
		
		log_verbose("Grew quad vbo to %d bytes.", d3d11_quad_vbo_size);
	}

	if (number_of_quads > 0) {
		///
		// Render geometry from into vbo quad list
	    
		
		ID3D11ShaderResourceView *textures[32];
		ID3D11ShaderResourceView *last_texture = 0;
		u64 num_textures = 0;
		s8 last_texture_index = 0;
		
		D3D11_Vertex* head = (D3D11_Vertex*)d3d11_staging_quad_buffer;
		D3D11_Vertex* pointer = head;
		u64 number_of_rendered_quads = 0;
		
		
		
		tm_scope("Quad processing") {
			if (draw_frame.enable_z_sorting) tm_scope("Z sorting") {
				if (!sort_quad_buffer || (sort_quad_buffer_size < number_of_quads*sizeof(Draw_Quad))) {
					// #Memory #Heapalloc
					if (sort_quad_buffer) dealloc(get_heap_allocator(), sort_quad_buffer);
					sort_quad_buffer = alloc(get_heap_allocator(), number_of_quads*sizeof(Draw_Quad));
					sort_quad_buffer_size = number_of_quads*sizeof(Draw_Quad);
				}
				radix_sort(draw_frame.quad_buffer, sort_quad_buffer, number_of_quads, sizeof(Draw_Quad), offsetof(Draw_Quad, z), MAX_Z_BITS);
			}
		
			for (u64 i = 0; i < number_of_quads; i++)  {
				
				Draw_Quad *q = &draw_frame.quad_buffer[i];
				
				assert(q->z <= MAX_Z, "Z is too high. Z is %d, Max is %d.", q->z, MAX_Z);
				assert(q->z >= (-MAX_Z+1), "Z is too low. Z is %d, Min is %d.", q->z, -MAX_Z+1);
				
				s8 texture_index = -1;
				
				if (q->image) {
					
					if (last_texture == q->image->gfx_handle) {
						texture_index = last_texture_index;
					} else {
						// First look if texture is already bound
						for (u64 j = 0; j < num_textures; j++) {
							if (textures[j] == q->image->gfx_handle) {
								texture_index = (s8)j;
								break;
							}
						}
						// Otherwise use a new slot
						if (texture_index <= -1) {
							if (num_textures >= 32) {
								// If max textures reached, make a draw call and start over
								D3D11_MAPPED_SUBRESOURCE buffer_mapping;
								ID3D11DeviceContext_Map(d3d11_context, (ID3D11Resource*)d3d11_quad_vbo, 0, D3D11_MAP_WRITE_DISCARD, 0, &buffer_mapping);
								memcpy(buffer_mapping.pData, d3d11_staging_quad_buffer, number_of_rendered_quads*sizeof(D3D11_Vertex)*6);
								ID3D11DeviceContext_Unmap(d3d11_context, (ID3D11Resource*)d3d11_quad_vbo, 0);
								d3d11_draw_call(number_of_rendered_quads, textures, num_textures);
								head = (D3D11_Vertex*)d3d11_staging_quad_buffer;
								num_textures = 0;
								texture_index = 0;
								number_of_rendered_quads = 0;
								pointer = head;
							} else {
								texture_index = (s8)num_textures;
								num_textures += 1;
							}
						}
					}
					textures[texture_index] = q->image->gfx_handle;
					last_texture = q->image->gfx_handle;
					last_texture_index = texture_index;
				}
				
				// This is meant to fix the annoying artifacts that shows up when sampling from a large atlas
			    // presumably for floating point precision issues or something.
			
			    // #Incomplete
			    // If we want to animate text with small movements then it will look wonky.
			    // This should be optional probably.
			
				float pixel_width = 2.0/(float)window.width;
				float pixel_height = 2.0/(float)window.height;

                bool xeven = window.width % 2 == 0;
                bool yeven = window.height % 2 == 0;
				
				q->bottom_left.x  = round(q->bottom_left.x  / pixel_width)  * pixel_width;
			    q->bottom_left.y  = round(q->bottom_left.y  / pixel_height) * pixel_height;
			    q->top_left.x     = round(q->top_left.x     / pixel_width)  * pixel_width;
			    q->top_left.y     = round(q->top_left.y     / pixel_height) * pixel_height;
			    q->top_right.x    = round(q->top_right.x    / pixel_width)  * pixel_width;
			    q->top_right.y    = round(q->top_right.y    / pixel_height) * pixel_height;
			    q->bottom_right.x = round(q->bottom_right.x / pixel_width)  * pixel_width;
			    q->bottom_right.y = round(q->bottom_right.y / pixel_height) * pixel_height;
				
				// We will write to 6 vertices for the one quad (two tris)
				 {
				
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
					
					
					if (q->image) {

						BL->uv = v2(q->uv.x1, q->uv.y1);
						TL->uv = v2(q->uv.x1, q->uv.y2);
						TR->uv = v2(q->uv.x2, q->uv.y2);
						BR->uv = v2(q->uv.x2, q->uv.y1);
						// #Hack #Bug #Cleanup
						// When a window dimension is uneven it slightly under/oversamples on an axis by a
						// seemingly arbitrary amount. The 0.25 is a magic value I got from trial and error.
						// (It undersamples by a fourth of the atlas texture?)
						// Anything > 0.25 < will slightly over/undersample on my machine.
						// I have no idea about #Portability here.
						// - Charlie M 26th July 2024
						if (window.width % 2 != 0) {
							BL->uv.x += (2.0/(float)q->image->width)*0.25;
							TL->uv.x += (2.0/(float)q->image->width)*0.25;
							TR->uv.x += (2.0/(float)q->image->width)*0.25;
							BR->uv.x += (2.0/(float)q->image->width)*0.25;
						}
						if (window.height % 2 != 0) {
							BL->uv.y -= (2.0/(float)q->image->height)*0.25;
							TL->uv.y -= (2.0/(float)q->image->height)*0.25;
							TR->uv.y -= (2.0/(float)q->image->height)*0.25;
							BR->uv.y -= (2.0/(float)q->image->height)*0.25;
						}

						u8 sampler = -1;
						if (q->image_min_filter == GFX_FILTER_MODE_NEAREST
									&& q->image_mag_filter == GFX_FILTER_MODE_NEAREST)
								sampler = 0;
						if (q->image_min_filter == GFX_FILTER_MODE_LINEAR
									&& q->image_mag_filter == GFX_FILTER_MODE_LINEAR)
								sampler = 1;
						if (q->image_min_filter == GFX_FILTER_MODE_LINEAR
									&& q->image_mag_filter == GFX_FILTER_MODE_NEAREST)
								sampler = 2;
						if (q->image_min_filter == GFX_FILTER_MODE_NEAREST
									&& q->image_mag_filter == GFX_FILTER_MODE_LINEAR)
								sampler = 3;
						BL->sampler=TL->sampler=TR->sampler=BR->sampler = (u8)sampler;
								
					}
					BL->texture_index=TL->texture_index=TR->texture_index=BR->texture_index = texture_index;
					
					BL->self_uv = v2(0, 0);
					TL->self_uv = v2(0, 1);
					TR->self_uv = v2(1, 1);
					BR->self_uv = v2(1, 0);
					
					// #Speed
					memcpy(BL->userdata, q->userdata, sizeof(q->userdata));
					memcpy(TL->userdata, q->userdata, sizeof(q->userdata));
					memcpy(TR->userdata, q->userdata, sizeof(q->userdata));
					memcpy(BR->userdata, q->userdata, sizeof(q->userdata));
					
					BL->color = TL->color = TR->color = BR->color = q->color;
					
					BL->type=TL->type=TR->type=BR->type = (u8)q->type;
					
					float t = q->scissor.y1;
					q->scissor.y1 = q->scissor.y2;
					q->scissor.y2 = t;
					
					q->scissor.y1 = window.pixel_height - q->scissor.y1;
					q->scissor.y2 = window.pixel_height - q->scissor.y2;
					
					BL->has_scissor=TL->has_scissor=TR->has_scissor=BR->has_scissor = q->has_scissor;
					BL->scissor=TL->scissor=TR->scissor=BR->scissor = q->scissor;
					
					*BL2 = *BL;
					*TR2 = *TR;
					
					number_of_rendered_quads += 1;
				}
			}
		}
		
		tm_scope("Write to gpu") {
		    D3D11_MAPPED_SUBRESOURCE buffer_mapping;
			tm_scope("The Map call") {
				hr = ID3D11DeviceContext_Map(d3d11_context, (ID3D11Resource*)d3d11_quad_vbo, 0, D3D11_MAP_WRITE_DISCARD, 0, &buffer_mapping);
			d3d11_check_hr(hr);
			}
			tm_scope("The memcpy") {
				memcpy(buffer_mapping.pData, d3d11_staging_quad_buffer, number_of_rendered_quads*sizeof(D3D11_Vertex)*6);
			}
			tm_scope("The Unmap call") {
				ID3D11DeviceContext_Unmap(d3d11_context, (ID3D11Resource*)d3d11_quad_vbo, 0);
			}
		}
		
		///
		// Draw call
		tm_scope("Draw call") d3d11_draw_call(number_of_rendered_quads, textures, num_textures);
    }
    
    reset_draw_frame(&draw_frame);
}

void gfx_update() {
	if (window.should_close) return;
	
	

	HRESULT hr;
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

	d3d11_process_draw_frame();

	tm_scope("Present") {
		IDXGISwapChain1_Present(d3d11_swap_chain, window.enable_vsync, window.enable_vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
	}
	
	
#if CONFIGURATION == DEBUG
	d3d11_output_debug_messages();
#endif
	
}


void gfx_init_image(Gfx_Image *image, void *initial_data) {

	void *data = initial_data;
    if (!initial_data){
    	// #Incomplete 8 bit width assumed
    	data = alloc(image->allocator, image->width*image->height*image->channels);
    	memset(data, 0, image->width*image->height*image->channels);
    }
    
	assert(image->channels > 0 && image->channels <= 4 && image->channels != 3, "Only 1, 2 or 4 channels allowed on images. Got %d", image->channels);

	D3D11_TEXTURE2D_DESC desc = ZERO(D3D11_TEXTURE2D_DESC);
	desc.Width = image->width;
	desc.Height = image->height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	switch (image->channels) {
		case 1: desc.Format = DXGI_FORMAT_R8_UNORM; break;
		case 2: desc.Format = DXGI_FORMAT_R8G8_UNORM; break;
		case 4: desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
		default: panic("You should not be here");
	}
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	
	D3D11_SUBRESOURCE_DATA data_desc = ZERO(D3D11_SUBRESOURCE_DATA);
	data_desc.pSysMem = data;
	data_desc.SysMemPitch  = image->width * image->channels;
	
	ID3D11Texture2D* texture = 0;
	HRESULT hr = ID3D11Device_CreateTexture2D(d3d11_device, &desc, &data_desc, &texture);
	d3d11_check_hr(hr);
	
	hr = ID3D11Device_CreateShaderResourceView(d3d11_device, (ID3D11Resource*)texture, 0, &image->gfx_handle);
	d3d11_check_hr(hr);
	
	if (!initial_data) {
		dealloc(image->allocator, data);
	}
	
	log_verbose("Created a D3D11 image of width %d and height %d.", image->width, image->height);
}
void gfx_set_image_data(Gfx_Image *image, u32 x, u32 y, u32 w, u32 h, void *data) {
    assert(image && data, "Bad parameters passed to gfx_set_image_data");

    ID3D11ShaderResourceView *view = image->gfx_handle;
    ID3D11Resource *resource = NULL;
    ID3D11ShaderResourceView_GetResource(view, &resource);
    
    assert(resource, "Invalid image passed to gfx_set_image_data");
    
    assert(x+w <= image->width && y+h <= image->height, "Specified subregion in image is out of bounds");

    ID3D11Texture2D *texture = NULL;
    HRESULT hr = ID3D11Resource_QueryInterface(resource, &IID_ID3D11Texture2D, (void**)&texture);
    assert(SUCCEEDED(hr), "Expected gfx resource to be a texture but it wasn't");

    D3D11_BOX destBox;
    destBox.left = x;
    destBox.right = x + w;
    destBox.top = y;
    destBox.bottom = y + h;
    destBox.front = 0;
    destBox.back = 1;

	// #Incomplete bit-width 8 assumed
    ID3D11DeviceContext_UpdateSubresource(d3d11_context, (ID3D11Resource*)texture, 0, &destBox, data, w * image->channels, 0);
}
void gfx_deinit_image(Gfx_Image *image) {
	ID3D11ShaderResourceView *view = image->gfx_handle;
	ID3D11Resource *resource = 0;
	ID3D11ShaderResourceView_GetResource(view, &resource);
	
	ID3D11Texture2D *texture = 0;
	HRESULT hr = ID3D11Resource_QueryInterface(resource, &IID_ID3D11Texture2D, (void**)&texture);
	if (SUCCEEDED(hr)) {
		D3D11Release(view);
		D3D11Release(texture);
		log("Destroyed an image");
	} else {
		panic("Unhandled D3D11 resource deletion");
	}
}

bool 
shader_recompile_with_extension(string ext_source, u64 cbuffer_size) {
	

	string source = string_replace_all(STR(d3d11_image_shader_source), STR("$INJECT_PIXEL_POST_PROCESS"), ext_source, get_temporary_allocator());
	
	
	if (!d3d11_compile_shader(source)) return false;
	
	u64 aligned_cbuffer_size = (max(cbuffer_size, 16) + 16) & ~(15);
	
	if (d3d11_cbuffer) {
		D3D11Release(d3d11_cbuffer);
	}
	D3D11_BUFFER_DESC desc = ZERO(D3D11_BUFFER_DESC);
	desc.ByteWidth      = aligned_cbuffer_size;
	desc.Usage          = D3D11_USAGE_DYNAMIC;
	desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = ID3D11Device_CreateBuffer(d3d11_device, &desc, null, &d3d11_cbuffer);
	d3d11_check_hr(hr);
	
	d3d11_cbuffer_size = cbuffer_size;
	
	return true;
}
    
    

const char *d3d11_image_shader_source = RAW_STRING(
	
struct VS_INPUT
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float2 self_uv : SELF_UV;
    float4 color : COLOR;
    int texture_index : TEXTURE_INDEX;
    uint type : TYPE;
    uint sampler_index : SAMPLER_INDEX;
    uint has_scissor : HAS_SCISSOR;
    float4 userdata[$VERTEX_2D_USER_DATA_COUNT] : USERDATA;
    float4 scissor : SCISSOR;
};

struct PS_INPUT
{
    float4 position_screen : SV_POSITION;
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
    float2 self_uv : SELF_UV;
    float4 color : COLOR;
    int texture_index: TEXTURE_INDEX;
    int type: TYPE;
    int sampler_index: SAMPLER_INDEX;
    uint has_scissor : HAS_SCISSOR;
    float4 userdata[$VERTEX_2D_USER_DATA_COUNT] : USERDATA;
    float4 scissor : SCISSOR;
};



PS_INPUT vs_main(VS_INPUT input)
{
    PS_INPUT output;
    output.position_screen = input.position;
    output.position = input.position;
    output.uv = input.uv;
    output.color = input.color;
    output.texture_index = input.texture_index;
    output.type          = input.type;
    output.sampler_index = input.sampler_index;
    output.self_uv = input.self_uv;
	for (int i = 0; i < $VERTEX_2D_USER_DATA_COUNT; i++) {
    	output.userdata[i] = input.userdata[i];
	}
	output.scissor = input.scissor;
	output.has_scissor = input.has_scissor;
    return output;
}

// #Magicvalue
Texture2D textures[32] : register(t0);
SamplerState image_sampler_0 : register(s0);
SamplerState image_sampler_1 : register(s1);
SamplerState image_sampler_2 : register(s2);
SamplerState image_sampler_3 : register(s3);

float4 sample_texture(int texture_index, int sampler_index, float2 uv) {
	// I love hlsl
	if (sampler_index == 0) {
		if (texture_index ==  0)       return textures[0].Sample(image_sampler_0, uv);
		else if (texture_index ==  1)  return textures[1].Sample(image_sampler_0, uv);
		else if (texture_index ==  2)  return textures[2].Sample(image_sampler_0, uv);
		else if (texture_index ==  3)  return textures[3].Sample(image_sampler_0, uv);
		else if (texture_index ==  4)  return textures[4].Sample(image_sampler_0, uv);
		else if (texture_index ==  5)  return textures[5].Sample(image_sampler_0, uv);
		else if (texture_index ==  6)  return textures[6].Sample(image_sampler_0, uv);
		else if (texture_index ==  7)  return textures[7].Sample(image_sampler_0, uv);
		else if (texture_index ==  8)  return textures[8].Sample(image_sampler_0, uv);
		else if (texture_index ==  9)  return textures[9].Sample(image_sampler_0, uv);
		else if (texture_index ==  10) return textures[10].Sample(image_sampler_0, uv);
		else if (texture_index ==  11) return textures[11].Sample(image_sampler_0, uv);
		else if (texture_index ==  12) return textures[12].Sample(image_sampler_0, uv);
		else if (texture_index ==  13) return textures[13].Sample(image_sampler_0, uv);
		else if (texture_index ==  14) return textures[14].Sample(image_sampler_0, uv);
		else if (texture_index ==  15) return textures[15].Sample(image_sampler_0, uv);
		else if (texture_index ==  16) return textures[16].Sample(image_sampler_0, uv);
		else if (texture_index ==  17) return textures[17].Sample(image_sampler_0, uv);
		else if (texture_index ==  18) return textures[18].Sample(image_sampler_0, uv);
		else if (texture_index ==  19) return textures[19].Sample(image_sampler_0, uv);
		else if (texture_index ==  20) return textures[20].Sample(image_sampler_0, uv);
		else if (texture_index ==  21) return textures[21].Sample(image_sampler_0, uv);
		else if (texture_index ==  22) return textures[22].Sample(image_sampler_0, uv);
		else if (texture_index ==  23) return textures[23].Sample(image_sampler_0, uv);
		else if (texture_index ==  24) return textures[24].Sample(image_sampler_0, uv);
		else if (texture_index ==  25) return textures[25].Sample(image_sampler_0, uv);
		else if (texture_index ==  26) return textures[26].Sample(image_sampler_0, uv);
		else if (texture_index ==  27) return textures[27].Sample(image_sampler_0, uv);
		else if (texture_index ==  28) return textures[28].Sample(image_sampler_0, uv);
		else if (texture_index ==  29) return textures[29].Sample(image_sampler_0, uv);
		else if (texture_index ==  30) return textures[30].Sample(image_sampler_0, uv);
		else if (texture_index ==  31) return textures[31].Sample(image_sampler_0, uv);
	} else if (sampler_index == 1) {
		if (texture_index ==  0)       return textures[0].Sample(image_sampler_1, uv);
		else if (texture_index ==  1)  return textures[1].Sample(image_sampler_1, uv);
		else if (texture_index ==  2)  return textures[2].Sample(image_sampler_1, uv);
		else if (texture_index ==  3)  return textures[3].Sample(image_sampler_1, uv);
		else if (texture_index ==  4)  return textures[4].Sample(image_sampler_1, uv);
		else if (texture_index ==  5)  return textures[5].Sample(image_sampler_1, uv);
		else if (texture_index ==  6)  return textures[6].Sample(image_sampler_1, uv);
		else if (texture_index ==  7)  return textures[7].Sample(image_sampler_1, uv);
		else if (texture_index ==  8)  return textures[8].Sample(image_sampler_1, uv);
		else if (texture_index ==  9)  return textures[9].Sample(image_sampler_1, uv);
		else if (texture_index ==  10) return textures[10].Sample(image_sampler_1, uv);
		else if (texture_index ==  11) return textures[11].Sample(image_sampler_1, uv);
		else if (texture_index ==  12) return textures[12].Sample(image_sampler_1, uv);
		else if (texture_index ==  13) return textures[13].Sample(image_sampler_1, uv);
		else if (texture_index ==  14) return textures[14].Sample(image_sampler_1, uv);
		else if (texture_index ==  15) return textures[15].Sample(image_sampler_1, uv);
		else if (texture_index ==  16) return textures[16].Sample(image_sampler_1, uv);
		else if (texture_index ==  17) return textures[17].Sample(image_sampler_1, uv);
		else if (texture_index ==  18) return textures[18].Sample(image_sampler_1, uv);
		else if (texture_index ==  19) return textures[19].Sample(image_sampler_1, uv);
		else if (texture_index ==  20) return textures[20].Sample(image_sampler_1, uv);
		else if (texture_index ==  21) return textures[21].Sample(image_sampler_1, uv);
		else if (texture_index ==  22) return textures[22].Sample(image_sampler_1, uv);
		else if (texture_index ==  23) return textures[23].Sample(image_sampler_1, uv);
		else if (texture_index ==  24) return textures[24].Sample(image_sampler_1, uv);
		else if (texture_index ==  25) return textures[25].Sample(image_sampler_1, uv);
		else if (texture_index ==  26) return textures[26].Sample(image_sampler_1, uv);
		else if (texture_index ==  27) return textures[27].Sample(image_sampler_1, uv);
		else if (texture_index ==  28) return textures[28].Sample(image_sampler_1, uv);
		else if (texture_index ==  29) return textures[29].Sample(image_sampler_1, uv);
		else if (texture_index ==  30) return textures[30].Sample(image_sampler_1, uv);
		else if (texture_index ==  31) return textures[31].Sample(image_sampler_1, uv);
	} else if (sampler_index == 2) {
		if (texture_index ==  0)       return textures[0].Sample(image_sampler_2, uv);
		else if (texture_index ==  1)  return textures[1].Sample(image_sampler_2, uv);
		else if (texture_index ==  2)  return textures[2].Sample(image_sampler_2, uv);
		else if (texture_index ==  3)  return textures[3].Sample(image_sampler_2, uv);
		else if (texture_index ==  4)  return textures[4].Sample(image_sampler_2, uv);
		else if (texture_index ==  5)  return textures[5].Sample(image_sampler_2, uv);
		else if (texture_index ==  6)  return textures[6].Sample(image_sampler_2, uv);
		else if (texture_index ==  7)  return textures[7].Sample(image_sampler_2, uv);
		else if (texture_index ==  8)  return textures[8].Sample(image_sampler_2, uv);
		else if (texture_index ==  9)  return textures[9].Sample(image_sampler_2, uv);
		else if (texture_index ==  10) return textures[10].Sample(image_sampler_2, uv);
		else if (texture_index ==  11) return textures[11].Sample(image_sampler_2, uv);
		else if (texture_index ==  12) return textures[12].Sample(image_sampler_2, uv);
		else if (texture_index ==  13) return textures[13].Sample(image_sampler_2, uv);
		else if (texture_index ==  14) return textures[14].Sample(image_sampler_2, uv);
		else if (texture_index ==  15) return textures[15].Sample(image_sampler_2, uv);
		else if (texture_index ==  16) return textures[16].Sample(image_sampler_2, uv);
		else if (texture_index ==  17) return textures[17].Sample(image_sampler_2, uv);
		else if (texture_index ==  18) return textures[18].Sample(image_sampler_2, uv);
		else if (texture_index ==  19) return textures[19].Sample(image_sampler_2, uv);
		else if (texture_index ==  20) return textures[20].Sample(image_sampler_2, uv);
		else if (texture_index ==  21) return textures[21].Sample(image_sampler_2, uv);
		else if (texture_index ==  22) return textures[22].Sample(image_sampler_2, uv);
		else if (texture_index ==  23) return textures[23].Sample(image_sampler_2, uv);
		else if (texture_index ==  24) return textures[24].Sample(image_sampler_2, uv);
		else if (texture_index ==  25) return textures[25].Sample(image_sampler_2, uv);
		else if (texture_index ==  26) return textures[26].Sample(image_sampler_2, uv);
		else if (texture_index ==  27) return textures[27].Sample(image_sampler_2, uv);
		else if (texture_index ==  28) return textures[28].Sample(image_sampler_2, uv);
		else if (texture_index ==  29) return textures[29].Sample(image_sampler_2, uv);
		else if (texture_index ==  30) return textures[30].Sample(image_sampler_2, uv);
		else if (texture_index ==  31) return textures[31].Sample(image_sampler_2, uv);
	} else if (sampler_index == 3) {
		if (texture_index ==  0)       return textures[0].Sample(image_sampler_3, uv);
		else if (texture_index ==  1)  return textures[1].Sample(image_sampler_3, uv);
		else if (texture_index ==  2)  return textures[2].Sample(image_sampler_3, uv);
		else if (texture_index ==  3)  return textures[3].Sample(image_sampler_3, uv);
		else if (texture_index ==  4)  return textures[4].Sample(image_sampler_3, uv);
		else if (texture_index ==  5)  return textures[5].Sample(image_sampler_3, uv);
		else if (texture_index ==  6)  return textures[6].Sample(image_sampler_3, uv);
		else if (texture_index ==  7)  return textures[7].Sample(image_sampler_3, uv);
		else if (texture_index ==  8)  return textures[8].Sample(image_sampler_3, uv);
		else if (texture_index ==  9)  return textures[9].Sample(image_sampler_3, uv);
		else if (texture_index ==  10) return textures[10].Sample(image_sampler_3, uv);
		else if (texture_index ==  11) return textures[11].Sample(image_sampler_3, uv);
		else if (texture_index ==  12) return textures[12].Sample(image_sampler_3, uv);
		else if (texture_index ==  13) return textures[13].Sample(image_sampler_3, uv);
		else if (texture_index ==  14) return textures[14].Sample(image_sampler_3, uv);
		else if (texture_index ==  15) return textures[15].Sample(image_sampler_3, uv);
		else if (texture_index ==  16) return textures[16].Sample(image_sampler_3, uv);
		else if (texture_index ==  17) return textures[17].Sample(image_sampler_3, uv);
		else if (texture_index ==  18) return textures[18].Sample(image_sampler_3, uv);
		else if (texture_index ==  19) return textures[19].Sample(image_sampler_3, uv);
		else if (texture_index ==  20) return textures[20].Sample(image_sampler_3, uv);
		else if (texture_index ==  21) return textures[21].Sample(image_sampler_3, uv);
		else if (texture_index ==  22) return textures[22].Sample(image_sampler_3, uv);
		else if (texture_index ==  23) return textures[23].Sample(image_sampler_3, uv);
		else if (texture_index ==  24) return textures[24].Sample(image_sampler_3, uv);
		else if (texture_index ==  25) return textures[25].Sample(image_sampler_3, uv);
		else if (texture_index ==  26) return textures[26].Sample(image_sampler_3, uv);
		else if (texture_index ==  27) return textures[27].Sample(image_sampler_3, uv);
		else if (texture_index ==  28) return textures[28].Sample(image_sampler_3, uv);
		else if (texture_index ==  29) return textures[29].Sample(image_sampler_3, uv);
		else if (texture_index ==  30) return textures[30].Sample(image_sampler_3, uv);
		else if (texture_index ==  31) return textures[31].Sample(image_sampler_3, uv);
	}
	
	return float4(1.0, 0.0, 0.0, 1.0);
}


\n

$INJECT_PIXEL_POST_PROCESS

\n
\043define QUAD_TYPE_REGULAR 0\n
\043define QUAD_TYPE_TEXT 1\n
\043define QUAD_TYPE_CIRCLE 2\n
float4 ps_main(PS_INPUT input) : SV_TARGET
{

	if (input.has_scissor) {
		float2 screen_pos = input.position_screen.xy;
		if (screen_pos.x < input.scissor.x || screen_pos.x >= input.scissor.z ||
			screen_pos.y < input.scissor.y || screen_pos.y >= input.scissor.w)
				discard;
	}

	if (input.type == QUAD_TYPE_REGULAR) {
		if (input.texture_index >= 0 && input.texture_index < 32 && input.sampler_index >= 0  && input.sampler_index <= 3) {
			return pixel_shader_extension(input, sample_texture(input.texture_index, input.sampler_index, input.uv)*input.color);
		} else {
			return pixel_shader_extension(input, input.color);
		}
	} else if (input.type == QUAD_TYPE_TEXT) {
		if (input.texture_index >= 0 && input.texture_index < 32 && input.sampler_index >= 0  && input.sampler_index <= 3) {
			float alpha = sample_texture(input.texture_index, input.sampler_index, input.uv).x;
			return pixel_shader_extension(input, float4(1.0, 1.0, 1.0, alpha)*input.color);
		} else {
			return pixel_shader_extension(input, input.color);
		}
	} else if (input.type == QUAD_TYPE_CIRCLE) {
	
		float dist = length(input.self_uv-float2(0.5, 0.5));
	
		if (dist > 0.5) return float4(0.0, 0.0, 0.0, 0.0);
	
		if (input.texture_index >= 0 && input.texture_index < 32 && input.sampler_index >= 0  && input.sampler_index <= 3) {
			return pixel_shader_extension(input, sample_texture(input.texture_index, input.sampler_index, input.uv)*input.color);
		} else {
			return pixel_shader_extension(input, input.color);
		}
	} 
	
	return float4(1.0, 1.0, 0.0, 1.0);
}
);