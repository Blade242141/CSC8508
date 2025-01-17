#include "Camera.h"

#include "ToonGame.h"
#include "ToonUtils.h"
#include "ToonFollowCamera.h"
#include "ToonMinimapCamera.h"
#include "ToonMapCamera.h"
#include "ToonObserverCamera.h"
#include "ToonRaycastCallback.h"
#include "PaintBallClass.h"
#include "ToonEventListener.h"
#include "InputManager.h"
#include "ToonDebugManager.h"
#include <Windows.h>
#include <Xinput.h>
#include "InputManager.h"
#include "XboxControllerInput.h"
#include "KeyboardInput.h"

using namespace NCL;
using namespace CSC8503;


ToonGame::ToonGame(GameTechRenderer* renderer, int playerCount, bool offline) : renderer(renderer), localPlayerCount(playerCount), offline(offline)
{
	AudioSystem::GetAudioSystem()->ApplyIngame();
	world = new ToonGameWorld();
	world->SetToonGame(this);

	ToonDebugManager::Instance().SetGameWorld(world);

	renderer->SetWorld(world);

	levelManager = new ToonLevelManager(world);
	world->AddEventListener(new ToonEventListener(&world->GetPhysicsWorld(), world, levelManager));
	baseWeapon = new PaintBallClass(world, levelManager, 15, 1000, 4.0f, 1.0f, 5);
	tieTeam = new Team("Draw", Vector3(1, 1, 1), 0);

	// If there are number of controllers equal to player count, use them, otherwise make P1 use keyboard
	for (int i = localPlayerCount; i > 0; i--) {
		XINPUT_STATE controllerState;
		if (XInputGetState(i - 1, &controllerState) == ERROR_SUCCESS)
			InputManager::GetInstance().AddInput(i, new XboxControllerInput(i - 1));
		else {
			for (int j = i; j > 1; j--) {
				InputManager::GetInstance().AddInput(j, new XboxControllerInput(j - 2));
			}
			InputManager::GetInstance().AddInput(1, new KeyboardInput(Window::GetKeyboard(), Window::GetMouse()));
			break;
		}
	}

	StartGame();
}

ToonGame::~ToonGame()
{
	delete world;
	delete baseWeapon;
	delete levelManager;
	for (auto& [id, control] : playerControls)
		delete control;
	delete tieTeam;
}

void ToonGame::StartGame() {
	gameTime = 75.0f;
	winner = nullptr;
	allPlayers.clear();
	world->MapNeedsChecking(true);
	world->GameStarted();
	for (auto& [id, control] : playerControls)
		delete control;
	playerControls.clear();
	if (offline) {
		levelManager->ResetLevel();
		world->SetNetworkStatus(NetworkingStatus::Offline);
		for (int i = 1; i <= localPlayerCount; i++) {
			Player* player = levelManager->AddPlayerToWorld(world->GetTeamLeastPlayers() /*world->GetTeams()[4]*/);
			players[i] = player;
			allPlayers.emplace(player);
			playerControls[i] = new PlayerControl();
			player->SetWeapon(baseWeapon);
			if (localPlayerCount == 1)
			{
				world->SetMinimapCamera(new ToonMinimapCamera(*player));
				
				PlayerNPC* playerNPC = levelManager->AddPlayerNPCToWorld(world->GetTeamLeastPlayers());
				playerNPC->SetWeapon(baseWeapon);
				allPlayers.emplace(playerNPC);
			}

			ToonFollowCamera* followCamera = new ToonFollowCamera(world, player, (localPlayerCount > 1 ? InputManager::GetInstance().GetFOV() + 10.0f : InputManager::GetInstance().GetFOV()));
			world->SetMainCamera(i, followCamera);
			player->SyncCamerasToSpawn(followCamera, playerControls[i]);
		}
	}
	else {
		levelManager->ResetLevel();
		world->SetMainCamera(1, new ToonObserverCamera());
	}
	world->SetMapCamera(new ToonMapCamera());
	renderer->ResetAtomicBuffer();
	accumulator = 0.0f;
}

void ToonGame::UpdateGame(float dt) {
	if (CheckDebugKeys()) return;

	world->UpdateWorld(dt);

	for (auto& [id, player] : players) {
		if (player && winner == nullptr) {
			UpdateCameras(dt, id);
			InputManager::GetInstance().GetInputs()[id]->UpdateGameControls(playerControls[id], world->GetMainCamera(id));
			player->WeaponUpdate(dt, playerControls[id]);
			if (offline) {
				player->MovementUpdate(dt, playerControls[id]);
			}
			else {
				player->SetAiming(playerControls[id]->aiming);
				if(playerControls[id]->aiming) { player->GetWeapon().UpdateTrajectory(dt, playerControls[id]); }
				else if (!playerControls[id]->aiming) { player->GetWeapon().HideTrajectory(); }
				
			}
		}
	}

	if (NetworkingStatus::Server)
		UpdateCameras(dt, 1);

	UpdatePhysics(dt);

	UpdateTime(dt);
	ShowUI(gameTime);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

PushdownState::PushdownResult ToonGame::DidSelectCancelButton()
{
	return PushdownState::Pop;
}

PushdownState::PushdownResult ToonGame::DidSelectOkButton()
{
	m_ShouldQuitGame = true;
	world->GameEnded();
	return PushdownState::Pop;
}

Player* NCL::CSC8503::ToonGame::GetPlayerFromID(const int& id)
{
	if(players.find(id) != players.end())
		return players[id];

	return NULL;
}

PushdownState::PushdownResult ToonGame::OnUpdate(float dt, PushdownState** newState)
{
	if (m_ShouldQuitGame)
	{
		world->MapNeedsChecking(true);
		ToonDebugManager::Instance().SetGameWorld(nullptr);
		return PushdownResult::Pop;
	}
	if (InputManager::GetInstance().GetInputs()[1]->IsBack() || closeGame)
	{
		if (offline)
		{
			*newState = GetToonConfirmationScreen();
			return PushdownResult::Push;
		}
	}
	//return PushdownResult::Pop;
	if (dt > 0.1f)
	{
		std::cout << "Skipping large time delta" << std::endl;
		return PushdownResult::NoChange; //must have hit a breakpoint or something to have a 1 second frame time!
	}
	UpdateGame(dt);

	return PushdownResult::NoChange;
}

void ToonGame::OnAwake()
{
	Window::GetWindow()->ShowOSPointer(false);
	Window::GetWindow()->LockMouseToWindow(true);
}

bool ToonGame::CheckDebugKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8) && offline) {
		StartGame();
		return true;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3)) {
		ToonDebugManager::Instance().ToggleCollisionDisplay();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Z)) {
		debugPlayerControl = 1;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::X)) {
		debugPlayerControl = 2;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9) && (offline || world->GetNetworkStatus() == NetworkingStatus::Server)) {
		gameTime = min(gameTime, 5.0f);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) renderer->ToggleDebug();
	return false;
}

void ToonGame::UpdateCameras(float dt, int localPlayer) {
	world->GetMainCamera(localPlayer)->UpdateCamera(dt, InputManager::GetInstance().GetInputs()[localPlayer]);
	if (localPlayer == 1)
		if (world->GetMinimapCamera())
			world->GetMinimapCamera()->UpdateCamera(dt, InputManager::GetInstance().GetInputs()[localPlayer]);
}

void ToonGame::UpdatePhysics(float dt) {
	ToonDebugManager::Instance().StartTimeCount("Physics");
	accumulator += dt;
	while (accumulator >= timeStep)
	{
		world->GetPhysicsWorld().update(reactphysics3d::decimal(timeStep));
		accumulator -= timeStep;
		world->DeleteMarkedObjects();
	}
	world->interpolationFactor = float(accumulator / timeStep);
	ToonDebugManager::Instance().EndTimeCount("Physics");
}

void ToonGame::UpdateTime(float dt) {
	gameTime -= dt;
	if (gameTime <= 0) {
		if (winner == nullptr && offline == true) {
			winner = DetermineWinner(renderer->GetTeamScores());
		}
		if (gameTime <= -5.0f && offline == true) {
			StartGame();
		}
	}
	else if (gameTime <= 74.7 && !renderer->IsMapInitialised()) {
		renderer->mapNeedsDrawing = true;
	}
}

void ToonGame::ShowUI(float time) {
	std::string output = "";
	if (time < 0) {
		output += '-';
		time *= -1;
	}
	int minutes = (int)time / 60;
	int seconds = (int)time % 60;
	output += to_string(minutes);
	output += ':';
	if (seconds < 10)
		output += "0";
	output += to_string(seconds);

	Debug::Print(output, NCL::Maths::Vector2(50.0f - output.size(), 5.0f));
	if (winner != nullptr)
	{
		const std::string winnerText = "WINNER:" + winner->GetTeamName();
		Debug::Print(winnerText, Vector2(50.0f - winnerText.size(), 15.0f), winner->GetTeamColour()); //TODO: Hardcoded for now. To be changed later.

		for (auto& player : allPlayers)
		{
			if (player->GetTeam() == winner) player->PlayVictory();
			else player->PlayDefeat();
		}
	}
}

Team* ToonGame::DetermineWinner(std::map<int, float> teamScores) {
	int currentWinner = 0;
	float winningScore = 0;
	bool tie = false;
	for (auto& [ID, score] : teamScores) {
		if (score > winningScore) {
			winningScore = score;
			currentWinner = ID;
			tie = false;
		}
		else if (score == winningScore)
			tie = true;
	}
	if (tie == true) return tieTeam;
	else return world->GetTeams().find(currentWinner)->second;
}

ToonConfirmationScreen* NCL::CSC8503::ToonGame::GetToonConfirmationScreen()
{
	if (!m_ToonConfirmationScreen)
	{
		m_ToonConfirmationScreen = new ToonConfirmationScreen(Coordinates(Vector2(30, 20), Vector2(50, 20)), m_WindowSize, renderer, "Are you sure, you want to quit the game?");
		m_ToonConfirmationScreen->delegate = this;
	}
	return m_ToonConfirmationScreen;
}