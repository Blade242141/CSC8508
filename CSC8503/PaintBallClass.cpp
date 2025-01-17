#include "PaintBallClass.h"
#include "PaintBallProjectile.h"
#include "ToonLevelManager.h"
#include "ToonRenderObject.h"
#include "ToonUtils.h"
#include "ToonRaycastCallback.h"
#include "ToonGameWorld.h"
#include "ToonLevelManager.h"
#include "Maths.h"

using namespace NCL;
using namespace CSC8503;

PaintBallClass::PaintBallClass(){}

PaintBallClass::PaintBallClass(ToonGameWorld* gameWorld, ToonLevelManager* levelManager, int _maxAmmoInUse, int _maxAmmoHeld, float _fireRate, float _reloadTime, float _maxShootDist) :
	gameWorld(gameWorld),
	levelManager(levelManager),
	ammoInUse(_maxAmmoInUse), 
	ammoHeld(_maxAmmoHeld), 
	maxAmmoInUse(_maxAmmoInUse), 
	maxAmmoHeld(_maxAmmoHeld),
	fireRate(_fireRate), 
	reloadTime(_reloadTime), 
	maxShootDistance(_maxShootDist)
{
	owningObject = nullptr;
	team = nullptr;

	shootTimer = 0.0f;
	reloadTimer = 0.0f;
	status = isIdle;
	for (int i = 0; i < trajectoryPoints; i++)
	{
		bullet[i] = nullptr;
	}

	shootSpread = 0.0f;
	shootSpreadAdd = 1.0f;
	shootSpreadMin = 0.0f;
	shootSpreadMax = 1.0f;

	trajectoryDetected = false;
}

PaintBallClass::~PaintBallClass() 
{
}

PaintBallClass PaintBallClass::MakeInstance() {
	return PaintBallClass(gameWorld, levelManager, maxAmmoInUse, maxAmmoHeld, fireRate, reloadTime, maxShootDistance);
}

//float PaintBallClass::GetYCoordinate(int x, int initialVelocity)
//{
//	return (float)(x * tan(gameWorld->GetMainCamera()->GetPitch()) - ((9.8 * x * x) / (2 * initialVelocity * initialVelocity * cos(gameWorld->GetMainCamera()->GetPitch()))));
//}

NCL::Maths::Vector3 NCL::CSC8503::PaintBallClass::CalculateBulletVelocity(NCL::Maths::Vector3 target, NCL::Maths::Vector3 origin, float t)
{
	Vector3 distance = target - origin;
	Vector3 distanceXZ = distance;
	distanceXZ.y = 0.0f;

	float sY = distance.y;
	float sXZ = distanceXZ.Length();

	float VxZ = sXZ * t;
	float Vy = (sY / t) + (0.5f * abs(gameWorld->GetPhysicsWorld().getGravity().y) * t);

	Vector3 result = distanceXZ.Normalised();
	result *= VxZ;
	result.y = Vy;

	return result;
}

void NCL::CSC8503::PaintBallClass::CalculateBulletPositionOrientation(const short& pitch, NCL::Maths::Vector3& positionFinal, NCL::Maths::Vector3& orientationFinal)
{
	reactphysics3d::Vector3 orientation = owningObject->GetRigidbody()->getTransform().getOrientation() * reactphysics3d::Quaternion::fromEulerAngles(reactphysics3d::Vector3((reactphysics3d::decimal(pitch + 5) / 180.0f * Maths::PI), 0, 0)) * reactphysics3d::Vector3(0, 0, -10.0f); // TODO: Update this to Sunit's new method of getting angle
	reactphysics3d::Vector3 dirOri = orientation;
	dirOri.y = 0;
	dirOri.normalize();
	orientation.normalize();
	reactphysics3d::Vector3 position = owningObject->GetRigidbody()->getTransform().getPosition() + dirOri * reactphysics3d::decimal(3) + reactphysics3d::Vector3(0, reactphysics3d::decimal(owningObject->GetScale().y * 1.5), 0);

	positionFinal = ToonUtils::ConvertToNCLVector3(position);
	orientationFinal = ToonUtils::ConvertToNCLVector3(orientation);
}

bool PaintBallClass::Update(float dt, PlayerControl* playerControls) 
{
	if (shootTimer > 0) shootTimer -= dt;
	if (playerControls->shooting && ammoInUse > 0 && shootTimer <= 0)
		status = isFiring;
	else if (ammoInUse <= 0 || Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::R))
		status = isReloading;
	else
		status = isIdle;

	if (shootSpread > 0) shootSpread -= dt * 2.0f;
	shootSpread = Clamp(shootSpread, shootSpreadMin, shootSpreadMax);

	switch (status) 
	{
		case isFiring:
				if (gameWorld->GetNetworkStatus() == NetworkingStatus::Offline) {					
					Vector3 position, orientation;
					CalculateBulletPositionOrientation(playerControls->camera[0] / 72.0f, position, orientation);
					FireBullet(ToonUtils::ConvertToRP3DVector3(position), ToonUtils::ConvertToRP3DVector3(orientation));
				}
				return true;
		case isReloading:
			Reload(dt);
		case isIdle:
			__fallthrough;
		default:
			return false;
	}
}

//Used by PlayerNPC
void NCL::CSC8503::PaintBallClass::NPCUpdate(float dt)
{
	if (gameWorld->GetNetworkStatus() != NetworkingStatus::Offline)
		return;

	if (shootTimer > 0) shootTimer -= dt;

	if (ammoInUse > 0 && shootTimer <= 0)
	{
		status = isFiring;
		Vector3 position, orientation;
		CalculateBulletPositionOrientation(ToonUtils::RandF(-45.0f, 5.0f), position, orientation);
		FireBullet(ToonUtils::ConvertToRP3DVector3(position), ToonUtils::ConvertToRP3DVector3(orientation));
	}
	else if (ammoInUse <= 0)
	{
		status = isReloading;
		Reload(dt);
	}
}

//void PaintBallClass::DrawTrajectory(NCL::Maths::Vector3 force)
//{
//	const float PAINTBALL_RADIUS = 0.25f;
//	const float PAINTBALL_IMPACT_RADIUS = 2.5f;
//	reactphysics3d::Vector3 orientation = owningObject->GetRigidbody()->getTransform().getOrientation() * reactphysics3d::Quaternion::fromEulerAngles(reactphysics3d::Vector3((reactphysics3d::decimal(gameWorld->GetMainCamera()->GetPitch() + 10) / 180.0f * std::acos(0.0)), 0, 0)) * reactphysics3d::Vector3(0, 0, -10.0f);;
//	orientation.normalize();
//	reactphysics3d::Vector3 position	= owningObject->GetRigidbody()->getTransform().getPosition() + orientation * 3 + reactphysics3d::Vector3(0, 1, 0);
//	//reactphysics3d::Vector3 forceVector = orientation * force;
//	reactphysics3d::Vector3 velocity	= ToonUtils::ConvertToRP3DVector3(force);
//	float flightDurartion				= (velocity.y) / -gameWorld->GetPhysicsWorld().getGravity().y;
//	float singlePointTime				= flightDurartion / trajectoryPoints;
//
//	if (flightDurartion < 0.0f) { HideTrajectory(); return; }
//	for (int i = 0; i < trajectoryPoints; i++)
//	{
//		float deltaTime = singlePointTime * i;
//		float x			= velocity.x * deltaTime;
//		float y			= velocity.y * deltaTime - (0.5f * -gameWorld->GetPhysicsWorld().getGravity().y * deltaTime * deltaTime);
//		float z		    = velocity.z * deltaTime;
//		position.x	   += x;
//		position.y	   += y;
//		position.z	   += z;
//
//		if (!bullet[i])
//		{
//			bullet[i] = levelManager->AddPaintBallProjectileToWorld(position, orientation, PAINTBALL_RADIUS, PAINTBALL_IMPACT_RADIUS, team);
//			bullet[i]->GetRigidbody()->setIsActive(false);
//		}
//		bullet[i]->SetPosition(position.x, position.y, position.z);
//	}
//}
void NCL::CSC8503::PaintBallClass::UpdateTrajectory(float dt, PlayerControl* playerControls)
{
	const float PAINTBALL_RADIUS		= 0.1f;
	const float PAINTBALL_IMPACT_RADIUS = 2.5f;
	reactphysics3d::Vector3 orientation = owningObject->GetRigidbody()->getTransform().getOrientation() * reactphysics3d::Quaternion::fromEulerAngles(reactphysics3d::Vector3((reactphysics3d::decimal((playerControls->camera[0] / 72.0f) + 5) / 180.0f * Maths::PI), 0, 0)) * reactphysics3d::Vector3(0, 0, -10.0f);
	orientation.normalize();
	reactphysics3d::Vector3 dirOri = orientation;
	dirOri.y = 0;
	dirOri.normalize();
	reactphysics3d::Vector3 position = owningObject->GetRigidbody()->getTransform().getPosition() + dirOri * reactphysics3d::decimal(3) + reactphysics3d::Vector3(0, reactphysics3d::decimal(owningObject->GetScale().y * 1.5), 0);
	reactphysics3d::Vector3 velocity = orientation * (m_ForceAppliedOnPaintBall * 0.1625f);
	float flightDurartion = (2 * velocity.y) / -gameWorld->GetPhysicsWorld().getGravity().y;
	float singlePointTime = 0.08f;//flightDurartion / trajectoryPoints;	//Hardcoding it so that singlePointTime don't become 0, due to flightDuration

	for (int i = 0; i < trajectoryPoints; i++)
	{
		float deltaTime = singlePointTime * i;
		float x			= velocity.x * deltaTime;
		float y			= velocity.y * deltaTime - (0.5f * -gameWorld->GetPhysicsWorld().getGravity().y * deltaTime * deltaTime);
		float z			= velocity.z * deltaTime;

		if (!bullet[i])
		{
			bullet[i] = levelManager->AddPaintBallProjectileToWorld(position, orientation, PAINTBALL_RADIUS, PAINTBALL_IMPACT_RADIUS, team, "NoShadow");
			bullet[i]->GetRigidbody()->setIsActive(false);
			bullet[i]->GetRigidbody()->enableGravity(true);
		}
		bullet[i]->SetPosition(position.x + x, position.y + y, position.z + z);
		Vector4 trajectoryBallColour = team->GetTeamColour();
		trajectoryBallColour.w = 0.8;
		bullet[i]->GetRenderObject()->SetColour(trajectoryBallColour);
	}
}

void PaintBallClass::HideTrajectory()
{
	for (int i = 0; i < trajectoryPoints; i++)
	{
		if (bullet[i])
		{
			bullet[i]->SetPosition(Vector3(-100, -100, -100));
		}
	}
}

void PaintBallClass::Reload(float dt) 
{
	if (ammoInUse < maxAmmoInUse && ammoHeld > 0) 
	{
		reloadTimer += dt;
		if (reloadTimer >= reloadTime) 
		{
			//Reload Maths here
			if ((ammoHeld + ammoInUse) < maxAmmoInUse) {
				ammoInUse += ammoHeld;
				ammoHeld = 0;
			}
			else {
				int temp = maxAmmoInUse - ammoInUse;
				ammoInUse += temp;
				ammoHeld -= temp;
			}
			//std::cout << "Ammo: " << ammoInUse << "/" << ammoHeld << std::endl;
		}
	}
}

void PaintBallClass::PickUpAmmo(int amt) {

}

void PaintBallClass::FireBullet(reactphysics3d::Vector3 position, reactphysics3d::Vector3 orientation) 
{
	const float PAINTBALL_RADIUS = 0.25f;
	const float PAINTBALL_IMPACT_RADIUS = 2.5f;

	shootTimer = 1.0f / fireRate;
	ammoInUse--;
	shootSpread += shootSpreadAdd;

	PaintBallProjectile* bullet = levelManager->AddPaintBallProjectileToWorld(position, orientation, PAINTBALL_RADIUS, PAINTBALL_IMPACT_RADIUS, team);
	//bullet->GetRigidbody()->setLinearVelocity(ToonUtils::ConvertToRP3DVector3(bulletVelocity));
	bullet->GetRigidbody()->applyWorldForceAtCenterOfMass(orientation * m_ForceAppliedOnPaintBall); // TODO: The force can maybe be applied better
}