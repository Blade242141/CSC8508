#pragma once

#include "Window.h"
#include "PushdownMachine.h"
#include "PushdownState.h"
#include "ToonGame.h"
#include "ToonNetworkedGame.h"
#include "ToonGameWorld.h"
#include "ToonTextInput.h"
#include "ToonConfirmationScreen.h"

using namespace NCL;
using namespace CSC8503;

class ToonMainMenu : public ToonConfirmationScreen
{
	enum GameStates
	{
		PLAY,
		MULTIPLAY,
		SETTINGS,
		CREDITS,
		QUIT,
		LAUNCHASSERVER,
		LAUNCHASCLIENT,
		BACK,
		PLAYAFTERSERIPSET,
		CONFIRMATION
	};

	struct MenuCoordinates
	{
		Vector2 position;
		Vector2 size;
	};
	struct MenuDataStruct
	{
		std::string		text;
		MenuCoordinates coordinates;
		Vector4			colour;
	};

private:
	Vector2						m_MouseLastPosition		  = Vector2(0.0f, 0.0f);
	Window*						m_Window				  = NULL;
	bool						m_IsMousePointerVisible   = true;
	int							m_BaseCurrentSelectdIndex = 0;
	GameTechRenderer*			m_Renderer;
	ToonGame*					m_Game					  = NULL;
	ToonGameWorld*				m_World;
	ToonMainMenu*				m_SubMenuScreenObject	  = NULL;
	const Vector4				m_HoverColour			  = Debug::GREEN;
	const Vector4				m_NormalTextColour		  = Debug::BLUE;
	std::vector<MenuDataStruct> m_mainMenuData = {
																MenuDataStruct("Play",		 MenuCoordinates(Vector2(5, 30), Vector2(80, 10)), m_NormalTextColour),
																MenuDataStruct("Multi Play", MenuCoordinates(Vector2(5, 40), Vector2(80, 10)), m_NormalTextColour),
																MenuDataStruct("Settings",   MenuCoordinates(Vector2(5, 50), Vector2(80, 10)), m_NormalTextColour),
																MenuDataStruct("Credits",    MenuCoordinates(Vector2(5, 60), Vector2(80, 10)), m_NormalTextColour),
																MenuDataStruct("Quit",		 MenuCoordinates(Vector2(5, 70), Vector2(80, 10)), m_NormalTextColour)
	};
	std::vector<MenuDataStruct> m_SubMainMenuData = {
																MenuDataStruct("Launch As Server", MenuCoordinates(Vector2(5, 35), Vector2(80, 10)), m_NormalTextColour),
																MenuDataStruct("Launch As Client", MenuCoordinates(Vector2(5, 45), Vector2(80, 10)), m_NormalTextColour),
																MenuDataStruct("Back",			   MenuCoordinates(Vector2(5, 55), Vector2(80, 10)), m_NormalTextColour)
	};
	int							m_CurrentSelectedIndex = 0;

	ToonTextInput*				m_UserInputScreenObject = NULL;
	bool						m_HasUserInitiatedScreenNavigation = false;

	ToonConfirmationScreen*		m_ToonConfirmationScreen = NULL;
	bool						m_ShouldQuitGame		 = false;

public:
	ToonMainMenu(GameTechRenderer* renderer, ToonGameWorld* world, Window* win);
	ToonMainMenu(GameTechRenderer* renderer, std::vector<MenuDataStruct> menuData, int baseCurrentSelectedIndex, ToonGameWorld* world, Window* win);

private:
	PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	void OnAwake() override;
	void OnSleep() override;
	void DrawMainMenu();
	bool IsInside(Vector2 mouseCoordinates, MenuCoordinates singleMenuCoordinates);
	PushdownState::PushdownResult NavigateToScreen(PushdownState** newState);
	ToonMainMenu* GetSubMenuSceenObject();
	ToonTextInput* GetUserInputScreenObject();
	void UpdateMosePointerState(bool isVisible);
	void WakeMouseOnMovement();

	//Delegates
	PushdownState::PushdownResult DidSelectCancelButton() override;
	PushdownState::PushdownResult DidSelectOkButton() override;
};