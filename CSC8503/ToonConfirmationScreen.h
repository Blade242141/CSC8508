#pragma once
#include "PushdownMachine.h"
#include "PushdownState.h"
#include "ToonGameWorld.h"
#include "GameTechRenderer.h"
#include "Toon2DCoordinates.h"
#include "InputManager.h"

using namespace NCL;
using namespace CSC8503;

#define CONFIRMATION_TEXT		   "Are you sure?"
#define EMPTY_COORDINATES		   Coordinates(Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f))
#define CONFIRMATION_BUTTON_WIDTH  15.0f
#define CONFIRMATION_BUTTON_HEIGHT 5.0f

class ToonConfirmationScreen : public PushdownState
{
	enum ConfirmationButtonsType
	{
		NoneButton = 0,
		Ok,
		Cancel
	};

	typedef struct ButtonStruct
	{
		std::string				buttonText;
		Coordinates				buttonCoordinates;
		ConfirmationButtonsType identifier;

		ButtonStruct(std::string text, Coordinates coordinates, ConfirmationButtonsType id)
		{
			buttonText = text;
			buttonCoordinates = coordinates;
			identifier = id;
		}
	}Button;

	private:
		const Vector4			m_SelectedTextColour = Debug::GREEN;
		Coordinates				m_Coordinates;
		std::string				m_Text;
		Vector4					m_TextColour;
		Button					m_OkButton;
		Button					m_CancelButton;
		ConfirmationButtonsType m_CurrentSelectedButton;
		bool					m_IsMousePointerVisible = true;
		Maths::Vector2			m_WindowSize;
		GameTechRenderer*		m_Renderer;
		Maths::Vector2			m_MouseLastPosition = Maths::Vector2(0.0f, 0.0f);
		bool					m_ShowBackground	= true;

	public:
		ToonConfirmationScreen* delegate;
		bool					m_ShouldRenderUpdates = true;

	public:
		ToonConfirmationScreen();
		ToonConfirmationScreen(Coordinates coordinates,
							   Maths::Vector2 windowSize,
							   GameTechRenderer* renderer,
							   std::string text			= CONFIRMATION_TEXT,
							   Vector4 textColour		= Debug::WHITE,
							   std::string okButtonText = "YES",
							   std::string noButtonText = "NO");

		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
		void OnAwake() override;
		void OnSleep() override;

		//Protocol Methods
		virtual PushdownState::PushdownResult DidSelectOkButton() { return PushdownState::PushdownResult::NoChange; }
		virtual PushdownState::PushdownResult DidSelectCancelButton() { return PushdownState::PushdownResult::NoChange; }
		virtual void UpdateCall(float dt) {}
		void DrawScreen();
		void HandleMouse();
		void HandleKeyboard();

	private:
		void UpdateButtonsCoordinates();
		void DrawSingleButton(ConfirmationButtonsType buttonType);
		int Clamp(int value, int lowerBound, int upperBound);
		ConfirmationButtonsType GetConfirmationButtonTypeFromIntegerValue(int value);
		void UpdateMousePointerState(bool isVisible);
		void WakeMouseOnMovement();
		void DrawBackground();
		void PrintText(bool withBackground = true);
};