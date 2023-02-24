#pragma once

#include "GameTechRenderer.h"

#include "ToonLevelManager.h"
#include <reactphysics3d/reactphysics3d.h>
#include "PaintBallClass.h"
#include "ToonEventListener.h"
#include "PushdownState.h"
#include "PlayerControl.h"
#include "ToonConfirmationScreen.h"

namespace NCL
{
	namespace CSC8503
	{
		class ToonFollowCamera;
		class ToonMinimapCamera;

		class ToonMapCamera;
		class ToonGame : public ToonConfirmationScreen//PushdownState
		{
		public:
			ToonGame(GameTechRenderer* renderer, bool offline = true);
			~ToonGame();

			virtual void UpdateGame(float dt);

			//Delegates
			PushdownState::PushdownResult DidSelectCancelButton() override;
			PushdownState::PushdownResult DidSelectOkButton() override;

		protected:
			virtual void StartGame();
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void OnAwake() override;
			bool CheckDebugKeys();

			void UpdateCameras(float dt, int localPlayer);
			void UpdatePhysics(float dt);
			void UpdateAnimations(float dt);
			void UpdateTime(float dt);

			void ShowUI(float time);
			Team* DetermineWinner(std::map<int, float> teamScores);
			ToonConfirmationScreen* GetToonConfirmationScreen();

		protected:
			ToonFollowCamera* followCamera;
			Player*	player = nullptr;
			PlayerControl* playerControl = nullptr;

			GameTechRenderer* renderer;
			ToonGameWorld* world;
			ToonLevelManager* levelManager;
			PaintBallClass* baseWeapon;
			std::unordered_set<Player*> allPlayers;

			Team* tieTeam;
			Team* winner;
			float gameTime;
			bool closeGame = false;
			bool offline;
			const double timeStep = 1.0 / 60.0;
			double accumulator;
			bool					m_ShouldQuitGame		 = false;
			ToonConfirmationScreen* m_ToonConfirmationScreen = NULL;

			public:
				Vector2 m_WindowSize;
		};
	}
}