
// This is copypasted in bloom.hlsl and bloom_map.hlsl.
// You can do #include in hlsl shaders, but I wanted this example to be very simple to look at.
cbuffer some_cbuffer : register(b0) {
    float2 mouse_pos_screen; // In pixels
    float2 window_size;
}
float4 get_light_contribution(PS_INPUT input) {

	const float light_distance = 500; // We could pass this with userdata
	const float intensity = 0.75;
	float2 vertex_pos = input.position_screen.xy;  // In pixels
    vertex_pos.y = window_size.y-vertex_pos.y; // For some reason d3d11 inverts the Y here so we need to revert it

	// Simple linear attenuation based on distance
    float attenuation = ((1.0 - (length(mouse_pos_screen - vertex_pos) / light_distance)))*intensity;
    
    return float4(attenuation, attenuation, attenuation, 1.0);
}

float4 pixel_shader_extension(PS_INPUT input, float4 color) {
	// We want to output everything above 1.0

	color = color + get_light_contribution(input);
	
	return float4(
		max(color.r-1.0, 0),
		max(color.g-1.0, 0),
		max(color.b-1.0, 0),
		color.a
	);
}