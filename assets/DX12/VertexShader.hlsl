struct VSIn {
	float4 normal 	: NORMAL0;
	float4 position : POSITION0;
	float4 color	: COLOR0;
	float2 texCoord : TEXCOORD0;
};

struct VSOut {
	float4 position : SV_POSITION;
	float4 normal 	: NORMAL0;
	float4 color	: COLOR0;
	float2 texCoord : TEXCOORD0;
};

cbuffer TRANSLATION_NAME : register(b0) {
	float4 translate;
}

cbuffer DIFFUSE_TINT_NAME : register(b1) {
	float4 diffuseTint;
}

VSOut main(VSIn input) {
	VSOut output = (VSOut)0;
	output.position = input.position + translate;
	output.color = input.color;

	// #ifdef NORMAL
	 	output.normal = input.normal;
	// #endif

	// #ifdef TEXTCOORD
	 	output.texCoord = input.texCoord;
	// #endif

	return output;
}
