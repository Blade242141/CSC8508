#include "ToonMapCamera.h"
#include "Window.h"
#include "Maths.h"
#include "ToonUtils.h"
#include "ToonRaycastCallback.h"
#include <reactphysics3d/reactphysics3d.h>

NCL::CSC8503::ToonMapCamera::ToonMapCamera()
{
	int width = 1280;
	int height = 720;


	nearPlane = -1.0f;
	farPlane = 10000.0f;

	left = -width / 2.0;
	right = width / 2.0;
	top = height / 2.0;
	bottom = -height / 2.0;

	left = left / zoomFactor;
	right = right / zoomFactor;
	top = top / zoomFactor;
	bottom = bottom / zoomFactor;


	pitch = -90.0f;
	yaw = 0.0f;

	camType = CameraType::Orthographic;

	position = Vector3(17.0f, distanceFromFocus, -20.0f);

}


void NCL::CSC8503::ToonMapCamera::UpdateCamera(float dt)
{
	
	
}