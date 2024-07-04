struct VS_INPUT
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    int texture_index: TEXTURE_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    int texture_index: TEXTURE_INDEX;
};

PS_INPUT vs_main(VS_INPUT input)
{
    PS_INPUT output;
    output.position = input.position;
    output.uv = input.uv;
    output.color = input.color;
    output.texture_index = input.texture_index;
    return output;
}

// #Magicvalue
Texture2D textures[32] : register(t0);
SamplerState image_sampler : register(s0);

float4 sample_texture(int texture_index, float2 uv) {
	// #Portability sigh 
		if (texture_index ==  0)       return textures[0].Sample(image_sampler, uv);
		else if (texture_index ==  1)  return textures[1].Sample(image_sampler, uv);
		else if (texture_index ==  2)  return textures[2].Sample(image_sampler, uv);
		else if (texture_index ==  3)  return textures[3].Sample(image_sampler, uv);
		else if (texture_index ==  4)  return textures[4].Sample(image_sampler, uv);
		else if (texture_index ==  5)  return textures[5].Sample(image_sampler, uv);
		else if (texture_index ==  6)  return textures[6].Sample(image_sampler, uv);
		else if (texture_index ==  7)  return textures[7].Sample(image_sampler, uv);
		else if (texture_index ==  8)  return textures[8].Sample(image_sampler, uv);
		else if (texture_index ==  9)  return textures[9].Sample(image_sampler, uv);
		else if (texture_index ==  10) return textures[10].Sample(image_sampler, uv);
		else if (texture_index ==  11) return textures[11].Sample(image_sampler, uv);
		else if (texture_index ==  12) return textures[12].Sample(image_sampler, uv);
		else if (texture_index ==  13) return textures[13].Sample(image_sampler, uv);
		else if (texture_index ==  14) return textures[14].Sample(image_sampler, uv);
		else if (texture_index ==  15) return textures[15].Sample(image_sampler, uv);
		else if (texture_index ==  16) return textures[16].Sample(image_sampler, uv);
		else if (texture_index ==  17) return textures[17].Sample(image_sampler, uv);
		else if (texture_index ==  18) return textures[18].Sample(image_sampler, uv);
		else if (texture_index ==  19) return textures[19].Sample(image_sampler, uv);
		else if (texture_index ==  20) return textures[20].Sample(image_sampler, uv);
		else if (texture_index ==  21) return textures[21].Sample(image_sampler, uv);
		else if (texture_index ==  22) return textures[22].Sample(image_sampler, uv);
		else if (texture_index ==  23) return textures[23].Sample(image_sampler, uv);
		else if (texture_index ==  24) return textures[24].Sample(image_sampler, uv);
		else if (texture_index ==  25) return textures[25].Sample(image_sampler, uv);
		else if (texture_index ==  26) return textures[26].Sample(image_sampler, uv);
		else if (texture_index ==  27) return textures[27].Sample(image_sampler, uv);
		else if (texture_index ==  28) return textures[28].Sample(image_sampler, uv);
		else if (texture_index ==  29) return textures[29].Sample(image_sampler, uv);
		else if (texture_index ==  30) return textures[30].Sample(image_sampler, uv);
		else if (texture_index ==  31) return textures[31].Sample(image_sampler, uv);
	return float4(1.0, 0.0, 0.0, 1.0);
}

float4 ps_main(PS_INPUT input) : SV_TARGET
{
	if (input.texture_index >= 0) {
		return sample_texture(input.texture_index, input.uv);
	} else {
		return input.color;
	}
}