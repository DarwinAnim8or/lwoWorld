#pragma once
class Quaternion
{
public:
	Quaternion(float X, float Y, float Z, float W);
	~Quaternion();

	float x;
	float y;
	float z;
	float w;
};