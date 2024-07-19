
// PS_INPUT is defined in the default shader in gfx_impl_d3d11.c at the bottom of the file

// BEWARE std140 packing:
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
cbuffer some_cbuffer : register(b0) {
    float2 mouse_pos_screen; // In pixels
    float2 window_size;
}

float4 pixel_shader_extension(PS_INPUT input, float4 color) {
    const float light_distance = 500;

    float2 vertex_pos = input.position_screen.xy;  // In pixels
    vertex_pos.y = window_size.y-vertex_pos.y; // For some reason d3d11 inverts the Y here so we need to revert it

	// Simple linear attenuation based on distance
    float attenuation = 1.0 - (length(mouse_pos_screen - vertex_pos) / light_distance);
    
    float4 light = float4(attenuation, attenuation, attenuation, 1.0);

    return color * light;
}