//-----------------------------------------------------------------
// Main Game  File
// C++ Header - X.h - version v7_01					
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------

#include "Resource.h"	
#include "GameEngine.h"
#include "AbstractGame.h"
#include "CommonHeader.h"

class EscapeZone;
class RegularAgent;
class Hazard;
class Wall;
class FlowField;

//-----------------------------------------------------------------
// X Class																
//-----------------------------------------------------------------
class EmotionContagionSim : public AbstractGame, public Callable
{
public:				
	//---------------------------
	// Constructor(s) and Destructor
	//---------------------------
	EmotionContagionSim();

	virtual ~EmotionContagionSim();

	//---------------------------
	// Disabling copy/move constructors and assignment operators   
	//---------------------------
	EmotionContagionSim(const EmotionContagionSim& other) = delete;
	EmotionContagionSim(EmotionContagionSim&& other) noexcept = delete;
	EmotionContagionSim& operator=(const EmotionContagionSim& other) = delete;
	EmotionContagionSim& operator=(EmotionContagionSim&& other) noexcept = delete;

	//---------------------------
	// General Methods
	//---------------------------
	void Initialize(HINSTANCE hInstance) override;
	void Start() override;
	void End() override;
	void MouseButtonAction(bool isLeft, bool isDown, int x, int y, WPARAM wParam) override;
	void MouseWheelAction(int x, int y, int distance, WPARAM wParam) override;
	void MouseMove(int x, int y, WPARAM wParam) override;
	void CheckKeyboard() override;
	void KeyPressed(TCHAR cKey) override;
	void Paint(RECT rect) override;
	void Tick() override;
	
	void CallAction(Caller* callerPtr) override;

private:
	// -------------------------
	// Datamembers
	// -------------------------

	void InitializeRegularAgents();
	void Initialize3rdPartyAgent();
	void Reset();
	void ResetWithoutAuthority();

	Button* m_pRegularResetButton;
	Button* m_pNoAuthorityResetButton;
	Button* m_pButtonFFExit;
	std::vector<Button*> m_pButtonsFFRooms;
	int m_pVisibleFF = 0;

	EscapeZone* m_pExit;
	std::vector<RegularAgent*> m_pAgents;
	RegularAgent* m_pPickedAgent;
	std::vector<Hazard*> m_pHazards;
	std::vector<Wall*> m_pWalls;
	FlowField* m_pExitFlowField;
	std::vector<FlowField*> m_pRoomFlowFields;
	std::vector<Quad*> m_pRoomQuads;
	const int m_SideWindowWidth = 300;

	int m_TimeMultiplier = 1;

	const bool m_RunTests = false;
	const int m_DesiredTestsNumber = 0;
	int m_DoneTestsNumber = 0;
	float m_CurrentTestTime = 0.0f;

};
