
// PS_INPUT is defined in the default shader in gfx_impl_d3d11.c at the bottom of the file

// BEWARE std140 packing:
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
cbuffer some_cbuffer : register(b0) {
    float2 mouse_pos_screen; // In pixels
    float2 window_size;
}

#define ENEMY_FLASH 1


// This procedure is the "entry" of our extension to the shader
// It basically just takes in the resulting color and input from vertex shader, for us to transform it
// however we want.
float4 pixel_shader_extension(PS_INPUT input, float4 color) {
    
    float detail_type = input.userdata[0].x;
    
	if (detail_type == ENEMY_FLASH) {
		float flash_amount = input.userdata[0].y;
	
        float2 pos = input.self_uv - float2(0.5, 0.5);
	
		float2 corner_distance = abs(pos) - (float2(0.5, 0.5) - corner_radius);
		
		float dist = length(max(corner_distance, 0.0)) - corner_radius;
		float smoothing = 0.01;
		float mask = 1.0-smoothstep(0.0, smoothing, dist);
	
		color *= mask;
    }
    
	
    return color;
}