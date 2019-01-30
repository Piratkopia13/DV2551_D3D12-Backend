struct VSOut {
	float4 position : SV_POSITION;
	// float4 normal 	: NORMAL0;
	// float4 color	: COLOR0;
	// float2 texCoord : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState ss : register(s0);

cbuffer DIFFUSE_TINT_NAME : register(b1) {
	float4 diffuseTint;
}

float4 PSMain(VSOut input) : SV_TARGET0 {

// #ifdef DIFFUSE_SLOT
//     float4 color = tex.Sample(ss, input.texCoord);
// #else
// 	float4 color = float4(1.0, 1.0, 1.0, 1.0);
// #endif

	return float4(0.0, 1.0, 0.0, 1.0);
	// return color * float4(diffuseTint.rgb, 1.0);
}