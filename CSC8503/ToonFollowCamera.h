#pragma once
#include "Camera.h"
#include "GameTechRenderer.h"
#include "ToonGameObject.h"
#include "Player.h"
#include "BaseInput.h"

namespace NCL
{
	namespace CSC8503
	{
		class ToonGameWorld;
		class ToonFollowCamera : public Camera
		{
		public:
			ToonFollowCamera(ToonGameWorld* gameWorld, ToonGameObject* target, float fov = 45.0f);
			~ToonFollowCamera() {};

			virtual void UpdateCamera(float dt, BaseInput* inputs) override;

			float GetPitchOffset() const { return pitchOffset; }
			void SetPitchOffset(const float& newPitchOffset) { pitchOffset = newPitchOffset; }

			float GetFollowDistance() const { return requiredRayDistance; }
			void SetFollowDistance(const float& newDistance) { requiredRayDistance = newDistance; }

			Vector3 GetFollowOffset() const { return followOffset; }
			void SetFollowOffset(const Vector3& newOffset) { followOffset = newOffset; }

			Vector3 GetTargetOffset() const { return targetOffset; }
			void SetTargetOffset(const Vector3& newOffset) { targetOffset = newOffset; }

			Vector3 GetAimOffset() const { return aimOffset; }
			void SetAimOffset(const Vector3& newOffset) { aimOffset = newOffset; }

			float GetSmoothness() const { return smoothness; }
			void SetSmoothness(const float& newSmoothness) { smoothness = newSmoothness; }

			Vector3 GetPlayerPosition() const { return player->GetPosition(); }

			Vector4 GetPlayerTeamColour() const { return player->GetTeam()->GetTeamColour();}

			Quaternion GetPlayerRotation() const { return player->GetOrientation(); }

			Vector3 followOffset2;

		protected:
			ToonGameWorld* gameWorld;
			ToonGameObject* followTarget;
			Player* player;

		private:
			Vector3 followOffset, targetOffset, aimOffset;
			Vector3 refVel;
			Vector3 up, right, forward;			
			
			float requiredRayDistance, defaultRayDistance;
			float pitchOffset;
			float smoothness;
			float distanceThresholdMoving, distanceThresholdStanding;
			float startFOV, aimFOV, vFov, zoomSmoothess;

			bool reached;
		};
	}
}