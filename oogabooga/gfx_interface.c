#if GFX_RENDERER == GFX_RENDERER_D3D11
	#include <d3d11.h>
	#include <dxgi.h>
	#include <dxgi1_2.h>
	#include <d3dcompiler.h>
	#include <dxgidebug.h>
	#include <d3dcommon.h>
	typedef ID3D11ShaderResourceView * Gfx_Handle;
	typedef ID3D11RenderTargetView * Gfx_Render_Target_Handle;
	
#elif GFX_RENDERER == GFX_RENDERER_VULKAN
	#error "We only have a D3D11 renderer at the moment"
#elif GFX_RENDERER == GFX_RENDERER_METAL
	#error "We only have a D3D11 renderer at the moment"
#else
	#error "Unknown renderer GFX_RENDERER defined"
#endif


#ifndef VERTEX_2D_USER_DATA_COUNT
	#define VERTEX_2D_USER_DATA_COUNT 1
#endif

ogb_instance const Gfx_Handle GFX_INVALID_HANDLE;
// #Volatile reflected in 2D batch shader
#define QUAD_TYPE_REGULAR 0
#define QUAD_TYPE_TEXT 1
#define QUAD_TYPE_CIRCLE 2

typedef enum Gfx_Filter_Mode {
	GFX_FILTER_MODE_NEAREST,
	GFX_FILTER_MODE_LINEAR,
} Gfx_Filter_Mode;

typedef struct Gfx_Image {
	u32 width, height, channels;
	Gfx_Handle gfx_handle;
	Gfx_Render_Target_Handle gfx_render_target;
	Allocator allocator;
} Gfx_Image;

typedef struct Draw_Frame Draw_Frame;

// Implemented per renderer
ogb_instance void gfx_render_draw_frame(Draw_Frame *frame, Gfx_Image *render_target);
ogb_instance void gfx_render_draw_frame_to_window(Draw_Frame *frame);
ogb_instance void gfx_init_image(Gfx_Image *image, void *data, bool render_target);
ogb_instance void gfx_set_image_data(Gfx_Image *image, u32 x, u32 y, u32 w, u32 h, void *data);
ogb_instance void gfx_read_image_data(Gfx_Image *image, u32 x, u32 y, u32 w, u32 h, void *output);
ogb_instance void gfx_deinit_image(Gfx_Image *image);
ogb_instance void gfx_init();
ogb_instance void gfx_update();
ogb_instance void gfx_reserve_vbo_bytes(u64 number_of_bytes);
ogb_instance bool gfx_shader_recompile_with_extension(string ext_source, u64 cbuffer_size);

DEPRECATED(bool shader_recompile_with_extension(string ext_source, u64 cbuffer_size), "Use gfx_shader_recompile_with_extension");


// initial_data can be null to leave image data uninitialized
Gfx_Image *make_image(u32 width, u32 height, u32 channels, void *initial_data, Allocator allocator) {
	// This is annoying but I did this long ago because stuff was a bit different and now I can't really change it :(
	Gfx_Image *image = alloc(allocator, sizeof(Gfx_Image));
	
	assert(channels > 0 && channels <= 4, "Only 1, 2, 3 or 4 channels allowed on images. Got %d", channels);
	
    image->width = width;
    image->height = height;
    image->allocator = allocator;
    image->channels = channels;
    
    gfx_init_image(image, initial_data, false);
    
    return image;
}

Gfx_Image *make_image_render_target(u32 width, u32 height, u32 channels, void *initial_data, Allocator allocator) {
	// This is annoying but I did this long ago because stuff was a bit different and now I can't really change it :(
	Gfx_Image *image = alloc(allocator, sizeof(Gfx_Image));
	
	assert(channels > 0 && channels <= 4, "Only 1, 2, 3 or 4 channels allowed on images. Got %d", channels);
	
    image->width = width;
    image->height = height;
    image->allocator = allocator;
    image->channels = channels;
    
    gfx_init_image(image, initial_data, true);
    
    return image;
}

Gfx_Image *load_image_from_disk(string path, Allocator allocator) {
    string png;
    bool ok = os_read_entire_file(path, &png, allocator);
    if (!ok) return 0;

    Gfx_Image *image = alloc(allocator, sizeof(Gfx_Image));
    
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    third_party_allocator = allocator;
    unsigned char* stb_data = stbi_load_from_memory(png.data, png.count, &width, &height, &channels, STBI_rgb_alpha);
    
    
    if (!stb_data) {
        dealloc(allocator, image);
        dealloc_string(allocator, png);
        return 0;
    }
    
    image->width = width;
    image->height = height;
    image->gfx_handle = GFX_INVALID_HANDLE;  // This is handled in gfx
    image->allocator = allocator;
    image->channels = 4;

    dealloc_string(allocator, png);
    
    gfx_init_image(image, stb_data, false);
    
    stbi_image_free(stb_data);
    
    third_party_allocator = ZERO(Allocator);

    return image;
}

void 
delete_image(Gfx_Image *image) {
      // Free the image data allocated by stb_image
    image->width = 0;
    image->height = 0;
    gfx_deinit_image(image);
    dealloc(image->allocator, image);
}
