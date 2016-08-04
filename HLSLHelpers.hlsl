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