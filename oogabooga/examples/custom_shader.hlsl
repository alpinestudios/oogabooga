
// PS_INPUT is defined in the default shader in gfx_impl_d3d11.c at the bottom of the file

// BEWARE std140 packing:
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
cbuffer some_cbuffer : register(b0) {
    float2 mouse_pos_screen; // In pixels
    float2 window_size;
}

#define DETAIL_TYPE_ROUNDED_CORNERS 1
#define DETAIL_TYPE_OUTLINED 2

float4 get_light_contribution(PS_INPUT input) {

	const float light_distance = 500; // We could pass this with userdata

	float2 vertex_pos = input.position_screen.xy;  // In pixels
    vertex_pos.y = window_size.y-vertex_pos.y; // For some reason d3d11 inverts the Y here so we need to revert it

	// Simple linear attenuation based on distance
    float attenuation = 1.0 - (length(mouse_pos_screen - vertex_pos) / light_distance);
    
    return float4(attenuation, attenuation, attenuation, 1.0);
}

// This procedure is the "entry" of our extension to the shader
// It basically just takes in the resulting color and input from vertex shader, for us to transform it
// however we want.
float4 pixel_shader_extension(PS_INPUT input, float4 color) {
    
    float detail_type = input.userdata[0].x;
    
	if (detail_type == DETAIL_TYPE_ROUNDED_CORNERS) {
		float corner_radius = input.userdata[0].y;
	
        float2 pos = input.self_uv - float2(0.5, 0.5);
	
		float2 corner_distance = abs(pos) - (float2(0.5, 0.5) - corner_radius);
		
		float dist = length(max(corner_distance, 0.0)) - corner_radius;
		float smoothing = 0.01;
		float mask = 1.0-smoothstep(0.0, smoothing, dist);
	
		color *= mask;
    } else if (detail_type == DETAIL_TYPE_OUTLINED) {
    	float line_width = input.userdata[0].y;
    	
    	float2 pixel_pos = round(input.self_uv*window_size);
        
        float xcenter = window_size.x/2;
        float ycenter = window_size.y/2;
        
        float xedge = pixel_pos.x < xcenter ? 0.0 : window_size.x;
        float yedge = pixel_pos.y < ycenter ? 0.0 : window_size.y;
        
        float xdist = abs(xedge-pixel_pos.x);
        float ydist = abs(yedge-pixel_pos.y);
        
        if (xdist >= line_width && ydist >= line_width) {
            discard;
        }
    }
    
	float4 light = get_light_contribution(input);
	
    return color * light;
}