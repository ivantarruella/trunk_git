

#include "functions.fx"

float g_SampleRadiusSSAO=0.023;
float g_DistanceScaleSSAO=0.405;

float4 SSAOPS(in float2 UV : TEXCOORD0) : COLOR
{
	
	float4 OUT=0;
	float4 samples[16] =
	{
		float4(0.355512, -0.709318, -0.102371, 0.0 ),
		float4(0.534186, 0.71511, -0.115167, 0.0 ),
		float4(-0.87866, 0.157139, -0.115167, 0.0 ),
		float4(0.140679, -0.475516, -0.0639818, 0.0 ),
		float4(-0.0796121, 0.158842, -0.677075, 0.0 ),
		float4(-0.0759516, -0.101676, -0.483625, 0.0 ),
		float4(0.12493, -0.0223423, -0.483625, 0.0 ),
		float4(-0.0720074, 0.243395, -0.967251, 0.0 ),
		float4(-0.207641, 0.414286, 0.187755, 0.0 ),
		float4(-0.277332, -0.371262, 0.187755, 0.0 ),
		float4(0.63864, -0.114214, 0.262857, 0.0 ),
		float4(-0.184051, 0.622119, 0.262857, 0.0 ),
		float4(0.110007, -0.219486, 0.435574, 0.0 ),
		float4(0.235085, 0.314707, 0.696918, 0.0 ),
		float4(-0.290012, 0.0518654, 0.522688, 0.0 ),
		float4(0.0975089, -0.329594, 0.609803, 0.0 )
	};
	
	//Tama�o renderizado pantalla
	float l_WidthScreenResolutionOffset=1/g_RenderTargetSize.x;
	float l_HeightScreenResolutionOffset=1/g_RenderTargetSize.y;
	
	
	float depth = tex2D(gDepthMapSampler, UV);
	float3 se=GetPositionFromZDepthViewInViewCoordinates(depth, UV);
	float4 vPositionVS = mul(float4(UV.x,UV.y,depth,1.0), g_InverseProj);
	depth=vPositionVS.z/vPositionVS.w;
	
	float3 randNormal = tex2D( gLightmapSampler, UV * 200.0 ).rgb;
	float finalColor = 0.0f;
	
	for (int i = 0; i < 16; i++)
	{
		float3 ray = reflect(samples[i].xyz,randNormal) * g_SampleRadiusSSAO;
		float4 sample = float4(se + ray, 1.0f);
		float4 ss = mul(sample, g_ProjMatrix);
		float2 sampleTexCoord = 0.5f * ss.xy/ss.w + float2(0.5f, 0.5f);
		sampleTexCoord.x += l_WidthScreenResolutionOffset;
		sampleTexCoord.y += l_HeightScreenResolutionOffset;
		sampleTexCoord.y=1.0-sampleTexCoord.y;
		float sampleDepth = tex2D(gDepthMapSampler, sampleTexCoord);
		vPositionVS = mul(float4(sampleTexCoord.x,sampleTexCoord.y,sampleDepth,1.0), g_InverseProj);
		sampleDepth=vPositionVS.z/vPositionVS.w;
		
		if (sampleDepth == 1.0)
		{
			finalColor ++;
		}
		else
		{
			//float occlusion = g_DistanceScaleSSAO* max(sampleDepth - depth,0.0f);
			float occlusion = g_DistanceScaleSSAO* abs(sampleDepth - depth);
			finalColor += 1.0f / (1.0f + occlusion * occlusion * 0.1);
		}
	}
	return float4(finalColor/16, finalColor/16, finalColor/16, 1.0f);
}


float4 SSAOFinalCompositionPS(in float2 UV : TEXCOORD0) : COLOR
{

	return float4(tex2D(gDiffuseSampler,UV).xyz *tex2D(	gLightmapSampler,UV).r,0.0);
}

float4 SSAOBlurPS(in float2 UV : TEXCOORD0) : COLOR
{
	float4 OUT=0;
	float l_RTWidth=g_RenderTargetSize.x;
	float l_RTHeight=g_RenderTargetSize.y;
	float2 blurDirection=float2(0.0, 1.0/l_RTHeight); //Vector Up screen
	UV.x += 1.0/l_RTWidth;
	UV.y += 1.0/l_RTHeight;
	float3 normal = tex2D(gNormalMapSampler, UV).rgb;
	normal=normalize(Texture2Normal(normal));
	float color = tex2D( gDiffuseSampler, UV).r;
	float num = 1;
	int blurSamples = 8;
	for( int i = -blurSamples/2; i <= blurSamples/2; i+=1)
	{
		float4 newTexCoord = float4(UV + i * blurDirection.xy, 0, 0);
		float sample = tex2D(gDiffuseSampler, newTexCoord).r;
		float3 samplenormal = tex2D(gNormalMapSampler, newTexCoord).rgb;
		samplenormal=normalize(Texture2Normal(samplenormal));
		if (dot(samplenormal, normal) > 0.99 )
		{
			num += (blurSamples/2 - abs(i));
			color += sample * (blurSamples/2 - abs(i));
		}
	}
	return color / num;
}

//Technique que genera la textura de SSAO
technique SSAOTechnique 
{
	pass p0
	{
		AlphaBlendEnable = false;
		CullMode = CCW;
		PixelShader = compile ps_3_0 SSAOPS();
	}
}
//Technique que renderiza el SSAO con la imagen completa
technique SSAOFinalCompositionTechnique
{
	pass p0
	{
		AlphaBlendEnable = false;
		CullMode = CCW;
		PixelShader = compile ps_3_0 SSAOFinalCompositionPS();
	}
}
//Tecnhnique con Blur (un efecto de emborronado antes de renderizar la �ltima pasada)
technique SSAOBlurTechnique
{
	pass p0
	{
		AlphaBlendEnable = false;
		CullMode = CCW;
		PixelShader = compile ps_3_0 SSAOBlurPS();
	}
}