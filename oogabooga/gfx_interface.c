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

forward_global const Gfx_Handle GFX_INVALID_HANDLE;

typedef struct Gfx_Image {
	u32 width, height, channels;
	Gfx_Handle gfx_handle;
	Allocator allocator;
} Gfx_Image;

Gfx_Image *make_image(u32 width, u32 height, u32 channels, void *initial_data, Allocator allocator);
Gfx_Image *load_image_from_disk(string path, Allocator allocator);
void delete_image(Gfx_Image *image);

void gfx_init_image(Gfx_Image *image, void *data);
void gfx_set_image_data(Gfx_Image *image, u32 x, u32 y, u32 w, u32 h, void *data);
void gfx_deinit_image(Gfx_Image *image);

#define FONT_ATLAS_WIDTH  4096
#define FONT_ATLAS_HEIGHT 4096
#define MAX_FONT_HEIGHT 256

typedef struct Gfx_Font_Metrics {
	u32 latin_ascent;
	u32 latin_descent;
	
	u32 max_ascent;
	u32 max_descent;
	
	u32 line_height;
	u32 line_spacing;
	
} Gfx_Font_Metrics;
typedef struct Gfx_Font_Atlas {
	Gfx_Image image;
	
} Gfx_Font_Atlas;
typedef struct Gfx_Font_Variation {
	u32 height;
	Gfx_Font_Metrics metrics;
	u32 codepoint_range_per_atlas;
	Hash_Table atlases; // u32 atlas_index, Gfx_Font_Atlas
	bool initted;
} Gfx_Font_Variation;
typedef struct Gfx_Font {
	Gfx_Font_Variation variations[MAX_FONT_HEIGHT]; // Variation per font height
	Allocator allocator;
} Gfx_Font;




// initial_data can be null to leave image data uninitialized
Gfx_Image *make_image(u32 width, u32 height, u32 channels, void *initial_data, Allocator allocator) {
	Gfx_Image *image = alloc(allocator, sizeof(Gfx_Image) + width*height*channels);
	
	assert(channels > 0 && channels <= 4, "Only 1, 2, 3 or 4 channels allowed on images. Got %d", channels);
	
    image->width = width;
    image->height = height;
    image->gfx_handle = GFX_INVALID_HANDLE;  // This is handled in gfx
    image->allocator = allocator;
    image->channels = 4;
    
    gfx_init_image(image, initial_data);
    
    return image;
}

Gfx_Image *load_image_from_disk(string path, Allocator allocator) {
    string png;
    bool ok = os_read_entire_file(path, &png, allocator);
    if (!ok) return 0;

    Gfx_Image *image = alloc(allocator, sizeof(Gfx_Image));
    
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
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
    
    gfx_init_image(image, stb_data);
    
    stbi_image_free(stb_data);

    return image;
}

void delete_image(Gfx_Image *image) {
      // Free the image data allocated by stb_image
    image->width = 0;
    image->height = 0;
    gfx_deinit_image(image);
    dealloc(image->allocator, image);
}
