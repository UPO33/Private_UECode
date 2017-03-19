/*
generates a hash value between 0;1 with two vectors
param _Location
param _Orientation
*/
{
	int3 v1 = (int3)(_Location * 1000);
	int hi1 = (((v1.x * 732756093) ^ (v1.y * 19349663) ^ (v1.z * 273492791)));

	int3 v2 = (int3)(_Orientation * 10000);
	int hi2 = (((v2.x * 732756093) ^ (v2.y * 19349663) ^ (v2.z * 273492791)));

	return abs((float)((hi1 ^ hi2) % 255)) / 255.0f;
}

/*
return sinusoidal wave between between 0;1 based on calculated fallof
*/
{
	if(_Value < _Radius || _Value > _Radius + _Fallof)
		return 0;
	else
		return sin(((_Value - _Radius) / _Fallof) * 3.141592f);
}

/*
returns linear wave between 0;1 base of calculated fallof
*/
{
	if(_Value < _Radius || _Value > _Radius + _Fallof)
		return 0;
	else
		return  1.0f - abs(lerp(-1.0, 1.0, (_Value - _Radius) / _Fallof));
}
/*
	Edge Detection Vertical && Horizontal
	
	@param _UV current pixel uv to smaple from
	@param _PixelSize 
	
*/
{
	//13 == CustomDepth, 24 == CustonStenciel, 27 == CustomGBuffer
	int sceneTextureID = 24;

	float3 sampleRowUp = float3(SceneTextureLookup(_UV + float2(-1,-1) * _PixelSize, sceneTextureID, false).r
			,SceneTextureLookup(_UV + float2(0,-1) * _PixelSize, sceneTextureID, false).r
			,SceneTextureLookup(_UV + float2(1,-1) * _PixelSize, sceneTextureID, false).r);
			
	float3 sampleRowDown = float3(SceneTextureLookup(_UV + float2(-1,1) * _PixelSize, sceneTextureID, false).r
			,SceneTextureLookup(_UV + float2(0,1) * _PixelSize, sceneTextureID, false).r
			,SceneTextureLookup(_UV + float2(1,1) * _PixelSize, sceneTextureID, false).r);
			
	float3 sampleColumnRight = float3(sampleRowUp.z, SceneTextureLookup(_UV + float2(1,0) * _PixelSize, sceneTextureID, false).r, sampleRowDown.z);

	float3 sampleColumnLeft = float3(sampleRowUp.x, SceneTextureLookup(_UV + float2(-1,0) * _PixelSize, sceneTextureID, false).r, sampleRowDown.x);

	float hr = dot(sampleRowUp, float3(-1, -2, -1)) + dot(sampleRowDown, float3(1, 2, 1));
	float vr = dot(sampleColumnLeft, float3(-1, -2, -1)) + dot(sampleColumnRight, float3(1, 2, 1));

	return length(float2(hr, vr));
}