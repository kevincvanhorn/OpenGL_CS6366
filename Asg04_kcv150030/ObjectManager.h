#pragma once
class Animation;

class ObjectManager
{
public:
	ObjectManager(Animation* InAnimation);

	void rotateLocalX();
	void rotateGlobalY();
	void resetModel();

	float RotAngleA;
	float RotAngleB;

	Animation* animation;
};

