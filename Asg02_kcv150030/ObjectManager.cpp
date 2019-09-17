#include "ObjectManager.h"
#include "Animation.h"

ObjectManager::ObjectManager(Animation* InAnimation) {
	RotAngleA = 45;
	RotAngleB = 45;
	animation = InAnimation;
}

void ObjectManager::rotateLocalX()
{
	if (!animation) return;
	animation->rotateLocalX(RotAngleA);
}

void ObjectManager::rotateGlobalY()
{
	if (!animation) return;
	animation->rotateGlobalY(RotAngleB);
}

void ObjectManager::resetModel()
{
	if (!animation) return;

	glm::mat4 trans = glm::mat4();
	trans = glm::translate(trans, glm::vec3(5.0, 0.0, 0.0));
	animation->setTransform(trans);
}
