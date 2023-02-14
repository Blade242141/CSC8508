#include "ToonMainMenu.h"

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
	//Debug::Print("------", Vector2(50, 20), Debug::CYAN);
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN))
	{
		m_MouseLastPosition = Window::GetMouse()->GetWindowPosition();
		UpdateMosePointerState(false);
		m_CurrentSelectedIndex = (m_CurrentSelectedIndex + 1) % m_mainMenuData.size();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP))
	{
		m_MouseLastPosition = Window::GetMouse()->GetWindowPosition();
		UpdateMosePointerState(false);
		m_CurrentSelectedIndex -= 1;
		if (m_CurrentSelectedIndex < 0) { m_CurrentSelectedIndex = (int)(m_mainMenuData.size()) - 1; }
	}

	if (!m_IsMousePointerVisible) { WakeMouseOnMovement(); }

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) { return PushdownResult::Pop; }	//Keeping it to quit game on escape key press

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN) || Window::GetMouse()->ButtonPressed(MouseButtons::LEFT))
	{
		return NavigateToScreen(newState);
	}

	Debug::Draw2DLine(Vector2(50, 20), Vector2(80, 20), Debug::RED);


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
	delete m_SubMenuScreenObject;
	m_SubMenuScreenObject = NULL;
}

void ToonMainMenu::OnSleep()
{

}

bool ToonMainMenu::IsInside(Vector2 mouseCoordinates, MenuCoordinates singleMenuCoordinates)
{
	float widthConstraint = singleMenuCoordinates.position.x + singleMenuCoordinates.size.x;
	float heightConstraint = singleMenuCoordinates.position.y + singleMenuCoordinates.size.y;
	return (mouseCoordinates.x >= singleMenuCoordinates.position.x && mouseCoordinates.x <= widthConstraint && mouseCoordinates.y >= singleMenuCoordinates.position.y && mouseCoordinates.y <= heightConstraint);
}


PushdownState::PushdownResult ToonMainMenu::NavigateToScreen(PushdownState** newState)
{
	switch (m_CurrentSelectedIndex + m_BaseCurrentSelectdIndex)
	{
	case PLAY:
		m_Game = new ToonGame(m_Renderer);
		*newState = m_Game;
		//m_mainMenuData[0].text = "Resume";
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
		return PushdownResult::NoChange;
	case LAUNCHASCLIENT:
		return PushdownResult::NoChange;
	case SETSERVERIP:
		return PushdownResult::NoChange;
	case BACK:
		return PushdownResult::Pop;
	}
	return PushdownResult::Push;
}

ToonMainMenu* ToonMainMenu::GetSubMenuSceenObject()
{
	if (!m_SubMenuScreenObject) { m_SubMenuScreenObject = new ToonMainMenu(m_Renderer, m_SubMainMenuData, (int)(m_mainMenuData.size()), m_World, m_Window); }
	return m_SubMenuScreenObject;
}

void ToonMainMenu::UpdateMosePointerState(bool isVisible)
{
	Window::GetWindow()->ShowOSPointer(isVisible);
	m_IsMousePointerVisible = isVisible;
}

void ToonMainMenu::WakeMouseOnMovement()
{
	Vector2 currentMousePosition = Window::GetMouse()->GetWindowPosition();
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
		Vector2 mousePosition = Window::GetMouse()->GetWindowPosition();
		Vector2 windowSize = m_Window->GetWindow()->GetScreenSize();
		float y = ((mousePosition.y / windowSize.y) * 100) + 5.0f;
		float x = ((mousePosition.x / windowSize.x) * 100) + 5.0f;
		Vector2 mousePositionWithinBounds = Vector2(x, y);
		int index = 0;
		for (auto data : m_mainMenuData)
		{
			data.colour = m_NormalTextColour;
			m_CurrentSelectedIndex = IsInside(mousePositionWithinBounds, data.coordinates) ? index : m_CurrentSelectedIndex;
			data.colour = index == m_CurrentSelectedIndex ? m_HoverColour : m_NormalTextColour;
			Debug::Print(data.text, data.coordinates.position, data.colour);
			index++;
		}
	}
}