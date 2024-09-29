
// This is copypasted in bloom.hlsl and bloom_map.hlsl.
// You can do #include in hlsl shaders, but I wanted this example to be very simple to look at.
cbuffer some_cbuffer : register(b0) {
    float2 mouse_pos_screen; // In pixels
    float2 window_size;
}

Texture2D bloom_map: register(t0); // 0 because we bound to 0 in bloom.c

/*
	Samplers:
	
	image_sampler_0 // near POINT,  far POINT
	image_sampler_1 // near LINEAR, far LINEAR
	image_sampler_2 // near POINT,  far LINEAR
	image_sampler_3 // near LINEAR, far POINT
	
	
	This is a oogabooga quirk at the moment. May get a better API at some point.

*/

float4 pixel_shader_extension(PS_INPUT input, float4 color) {
	const int BLUR_SAMPLE_RADIUS_PIXELS = 5;
	const float SAMPLES_PER_PIXEL = 1.0;
	const int BLUR_SAMPLE_RADIUS = BLUR_SAMPLE_RADIUS_PIXELS*SAMPLES_PER_PIXEL;
	
	// This makes d3d compiler very slow because dynamic length of loops. Ideally you would bake the window size into the shader source as a constant and recompile each time the window resized
	const float2 BLUR_OFFSET = float2(1.0 / window_size.x, 1.0/window_size.y) * 1.0/SAMPLES_PER_PIXEL;

    float4 total_bloom = float4(0, 0, 0, 0);
    
    float total_weight = 0.0;
    float radius_sq = float(BLUR_SAMPLE_RADIUS * BLUR_SAMPLE_RADIUS);

    for(int x = -BLUR_SAMPLE_RADIUS; x <= BLUR_SAMPLE_RADIUS; x++) {
        for(int y = -BLUR_SAMPLE_RADIUS; y <= BLUR_SAMPLE_RADIUS; y++) {
            float dist = (float)(x*x + y*y);
            
            if (dist < radius_sq) {
            	float2 sample_offset = float2(x, y) * BLUR_OFFSET;
	            float4 bloom = bloom_map.Sample(image_sampler_1, input.uv + sample_offset); // See comment on top of file for explanation of image_sampler_1
	            total_bloom += bloom;
	            total_weight += 1.0;
            }
            
        }
    }

    total_bloom /= total_weight;

	return color + total_bloom;
}