#pragma once
#include "ToonGameObject.h"
#include "Team.h"

using namespace NCL;
using namespace CSC8503;

class HitSphere : public ToonGameObject {
public:
	HitSphere(reactphysics3d::PhysicsWorld& RP3D_World, Team* team, reactphysics3d::Vector3 position, float radius /*, Weapon weapon*/);
	~HitSphere();

	void OnCollisionBegin(ToonGameObject* otherObject);

	void Update(float dt);

	Vector3 GetTeamColour() const { return teamColour; }

protected:
	float radius;
	Vector3 teamColour;

	float lifetime; //Time left before removed from game world.

	/*
	* Eventually when weapons are added:
	* 
	* float weaponRadius
	*/
};