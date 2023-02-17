#include "ToonTextInput.h"

ToonTextInput::ToonTextInput(Coordinates coordinates, GameTechRenderer* renderer, Vector2 windowSize, std::function<void(std::string)> doneButtonClosure, ToonVirtualKeyboard::KeyboardInputType inputType, Vector4 focusColour, Vector4 unfocusColour) : m_Coordinates(coordinates), m_Renderer(renderer), m_WindowSize(windowSize), m_doneButtonClosure(doneButtonClosure), m_InputType(inputType), m_IsFocused(true), m_FocusColour(focusColour), m_UnfocusColour(unfocusColour), m_InputText("")
{
	Coordinates keyboardCoordinates  = m_Coordinates;
	keyboardCoordinates.origin.y	+= keyboardCoordinates.size.y;
	m_VirtualKeyboard				 = new ToonVirtualKeyboard(keyboardCoordinates, m_WindowSize);
}

ToonTextInput::~ToonTextInput()
{
	delete m_VirtualKeyboard;
}

void ToonTextInput::UpdatePosition(Coordinates newCoordinates)
{
	m_Coordinates = newCoordinates;
}

void ToonTextInput::DrawUserInputText()
{
	m_InputText					 = m_VirtualKeyboard->GetUserInputText();
	Vector2 inputTextCoordinates = m_Coordinates.origin + Vector2(1.0f, m_Coordinates.size.y / 2);
	Debug::Print(m_InputText, inputTextCoordinates, Debug::GREEN);
}

std::string ToonTextInput::GetUserInputText()
{
	return m_InputText;
}

PushdownState::PushdownResult ToonTextInput::OnUpdate(float dt, PushdownState** newState)
{
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) { return PushdownResult::Pop; }
	if (m_IsFocused) { m_VirtualKeyboard->UpdateAndHandleInputEvents(); }

	m_Renderer->Update(dt);
	m_Renderer->Render();
	Debug::UpdateRenderables(dt);
	Debug::DrawQuad(m_Coordinates.origin, m_Coordinates.size, Debug::BLUE);
	DrawUserInputText();
	if (m_VirtualKeyboard->m_HasUserClickedDoneButton)
	{
		m_VirtualKeyboard->m_HasUserClickedDoneButton = false;
		m_doneButtonClosure(m_VirtualKeyboard->GetUserInputText());
		return PushdownState::PushdownResult::Pop;
	}
	return PushdownState::PushdownResult::NoChange;
}

void ToonTextInput::OnAwake() {}
void ToonTextInput::OnSleep() {}