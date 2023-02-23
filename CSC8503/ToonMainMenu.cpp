#include "ToonMainMenu.h"
#include <regex>

ToonMainMenu::ToonMainMenu(GameTechRenderer* renderer, ToonGameWorld* world, Window* win)
{
	m_Renderer = renderer;
	m_World = world;
	m_CurrentSelectedIndex = 0;
	m_Window = win;
}

ToonMainMenu::ToonMainMenu(GameTechRenderer* renderer, std::vector<MenuDataStruct> menuData, int baseCurrentSelectedIndex, ToonGameWorld* world, Window* win)
{
	m_Renderer = renderer;
	m_mainMenuData = menuData;
	m_BaseCurrentSelectdIndex = baseCurrentSelectedIndex;
	m_CurrentSelectedIndex = 0;
	m_Window = win;
	m_World = world;
}


PushdownState::PushdownResult ToonMainMenu::OnUpdate(float dt, PushdownState** newState)
{
	if (InputManager::GetInstance().GetInputs()[1]->IsPushingDown())
	{
		m_MouseLastPosition = InputManager::GetInstance().GetInputs()[1]->GetMousePosition();
		UpdateMosePointerState(false);
		m_CurrentSelectedIndex = (m_CurrentSelectedIndex + 1) % m_mainMenuData.size();
	}
	if (InputManager::GetInstance().GetInputs()[1]->IsPushingUp())
	{
		m_MouseLastPosition = InputManager::GetInstance().GetInputs()[1]->GetMousePosition();
		UpdateMosePointerState(false);
		m_CurrentSelectedIndex -= 1;
		if (m_CurrentSelectedIndex < 0) { m_CurrentSelectedIndex = (int)(m_mainMenuData.size()) - 1; }
	}

	if (!m_IsMousePointerVisible) { WakeMouseOnMovement(); }

	if (InputManager::GetInstance().GetInputs()[1]->IsBack()) { return PushdownResult::Pop; }	//Keeping it to quit game on escape key press

	if (InputManager::GetInstance().GetInputs()[1]->IsSelecting() || InputManager::GetInstance().GetInputs()[1]->IsShooting() || m_HasUserInitiatedScreenNavigation)
	{
		return NavigateToScreen(newState);
	}

	m_Renderer->SetWorld(m_World);
	m_Renderer->Update(dt);
	m_Renderer->Render();
	Debug::UpdateRenderables(dt);
	DrawMainMenu();
	return PushdownResult::NoChange;
}

void ToonMainMenu::OnAwake()
{
	UpdateMosePointerState(true);
	Window::GetWindow()->LockMouseToWindow(true);
	if (m_Game)
	{
		delete m_Game;
		m_Game = NULL;
	}
	if (m_SubMenuScreenObject)
	{
		delete m_SubMenuScreenObject;
		m_SubMenuScreenObject = NULL;
	}
}

void ToonMainMenu::OnSleep()
{

}

bool ToonMainMenu::IsInside(Vector2 mouseCoordinates, MenuCoordinates singleMenuCoordinates)
{
	float widthConstraint  = singleMenuCoordinates.position.x + singleMenuCoordinates.size.x;
	float heightConstraint = singleMenuCoordinates.position.y + singleMenuCoordinates.size.y;
	return (mouseCoordinates.x >= singleMenuCoordinates.position.x && mouseCoordinates.x <= widthConstraint && mouseCoordinates.y >= singleMenuCoordinates.position.y && mouseCoordinates.y <= heightConstraint);
}


PushdownState::PushdownResult ToonMainMenu::NavigateToScreen(PushdownState** newState)
{
	int navigationScreenIndex = m_CurrentSelectedIndex + (!m_HasUserInitiatedScreenNavigation ? m_BaseCurrentSelectdIndex : 0);
	m_HasUserInitiatedScreenNavigation = false;
	switch (navigationScreenIndex)
	{
	case PLAY:
		m_Game = new ToonGame(m_Renderer);
		*newState = m_Game;
		break;
	case MULTIPLAY:
		*newState = GetSubMenuSceenObject();
		break;
	case SETTINGS:
		return PushdownResult::NoChange;
	case CREDITS:
		return PushdownResult::NoChange;
	case QUIT:
		return PushdownResult::Pop;
	case LAUNCHASSERVER:
		m_Game	  = new ToonNetworkedGame(m_Renderer);
		*newState = m_Game;
		break;
	case LAUNCHASCLIENT:
		*newState = GetUserInputScreenObject();
		break;
	case BACK:
		return PushdownResult::Pop;
	case PLAYAFTERSERIPSET:
		std::vector<int> ipAddressVector = m_UserInputScreenObject->GetSeparatedIPAddressComponents();
		if (ipAddressVector.size() != 4) { return PushdownResult::NoChange; }
		m_Game							 = new ToonNetworkedGame(m_Renderer, ipAddressVector[0], ipAddressVector[1], ipAddressVector[2], ipAddressVector[3]);
		*newState						 = m_Game;
		break;
	}
	return PushdownResult::Push;
}

ToonMainMenu* ToonMainMenu::GetSubMenuSceenObject()
{
	if (!m_SubMenuScreenObject) { m_SubMenuScreenObject = new ToonMainMenu(m_Renderer, m_SubMainMenuData, (int)(m_mainMenuData.size()), m_World, m_Window); }
	return m_SubMenuScreenObject;
}

ToonTextInput* ToonMainMenu::GetUserInputScreenObject()
{
	if (!m_UserInputScreenObject) { m_UserInputScreenObject = new ToonTextInput(Coordinates(Vector2(35, 20), Vector2(30, 5)), m_Renderer, m_Window->GetScreenSize(),
		[&](std::string inputStringText, bool* showError)
		{
			std::cout << inputStringText << std::endl;
			bool isValidIPAddress = std::regex_match(inputStringText, std::regex("(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"));
			if (isValidIPAddress)
			{
				m_HasUserInitiatedScreenNavigation = true;
				m_CurrentSelectedIndex			   = GameStates::PLAYAFTERSERIPSET;
				*showError						   = false;
				return PushdownResult::Pop;
			}
			else
			{
				*showError = true;
				return PushdownResult::NoChange;
			}
			return PushdownResult::NoChange;
		}, "Please enter a valid IP Address.");
	}
	return m_UserInputScreenObject;
}

void ToonMainMenu::UpdateMosePointerState(bool isVisible)
{
	Window::GetWindow()->ShowOSPointer(isVisible);
	m_IsMousePointerVisible = isVisible;
}

void ToonMainMenu::WakeMouseOnMovement()
{
	Vector2 currentMousePosition = InputManager::GetInstance().GetInputs()[1]->GetMousePosition();
	if (currentMousePosition != m_MouseLastPosition) { UpdateMosePointerState(true); }
	m_MouseLastPosition = currentMousePosition;
}

void ToonMainMenu::DrawMainMenu()
{
	if (!m_IsMousePointerVisible)
	{
		int index = 0;
		for (auto data : m_mainMenuData)
		{
			data.colour = m_CurrentSelectedIndex == index ? m_HoverColour : m_NormalTextColour;
			Debug::Print(data.text, data.coordinates.position, data.colour);
			index++;
		}
	}
	else
	{
		Vector2 mousePosition			  = InputManager::GetInstance().GetInputs()[1]->GetMousePosition();
		Vector2 windowSize				  = m_Window->GetWindow()->GetScreenSize();
		float	y						  = ((mousePosition.y / windowSize.y) * 100) + 5.0f;
		float	x						  = ((mousePosition.x / windowSize.x) * 100) + 5.0f;
		Vector2 mousePositionWithinBounds = Vector2(x, y);
		int		index					  = 0;
		for (auto data : m_mainMenuData)
		{
			data.colour			   = m_NormalTextColour;
			m_CurrentSelectedIndex = IsInside(mousePositionWithinBounds, data.coordinates) ? index : m_CurrentSelectedIndex;
			data.colour			   = index == m_CurrentSelectedIndex ? m_HoverColour : m_NormalTextColour;
			Debug::Print(data.text, data.coordinates.position, data.colour);
			index++;
		}
	}
}