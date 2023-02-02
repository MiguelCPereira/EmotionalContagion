//-----------------------------------------------------------------
// Main Game File
// C++ Source - X.cpp - version v7_01
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "EmotionContagionSim.h"		
#include "EscapeZone.h"
#include "RegularAgent.h"
#include "ThirdPartyAgent.h"
#include "Hazard.h"
#include "Wall.h"
#include "FlowField.h"

//-----------------------------------------------------------------
// X methods																				
//-----------------------------------------------------------------

EmotionContagionSim::EmotionContagionSim()
{
	// nothing to create
}

EmotionContagionSim::~EmotionContagionSim()
{
	// nothing to destroy
}


void EmotionContagionSim::Initialize(HINSTANCE hInstance)
{
	// Set the required values
	AbstractGame::Initialize(hInstance);
	GAME_ENGINE->SetTitle(_T("Emotional Contagion Simulation"));					
	GAME_ENGINE->RunGameLoop(true);		
	
	// Set the optional values
	GAME_ENGINE->SetWidth(1024 + m_SideWindowWidth);
	GAME_ENGINE->SetHeight(768);
    GAME_ENGINE->SetFrameRate(50); 

	// Set the keys that the project needs to listen to
	//tstringstream buffer;
	//buffer << _T("KLMO");
	//buffer << (TCHAR) VK_LEFT;
	//buffer << (TCHAR) VK_RIGHT;
	//GAME_ENGINE->SetKeyList(buffer.str());
}

void EmotionContagionSim::Start()
{
	const auto sceneWidth = (GAME_ENGINE->GetWidth() - m_SideWindowWidth);

	// Initialize the regular reset button
	m_pRegularResetButton = new Button(_T("Reset"));
	m_pRegularResetButton->SetBounds(20, 20, 60, 30);
	m_pRegularResetButton->AddActionListener(this);
	m_pRegularResetButton->Show();

	// Initialize the no authority reset button
	m_pNoAuthorityResetButton = new Button(_T("Reset W/out Autorithy"));
	m_pNoAuthorityResetButton->SetBounds(sceneWidth - 20 - 160, 20, 160, 30);
	m_pNoAuthorityResetButton->AddActionListener(this);
	m_pNoAuthorityResetButton->Show();


	// Initialize the flowfield view buttons
	const auto startRowPos = Vector2f(sceneWidth / 2.f - 105, 20);
	m_pButtonFFExit = new Button(_T("E"));
	m_pButtonFFExit->SetBounds(startRowPos.X, startRowPos.Y, 30, 30);
	m_pButtonFFExit->AddActionListener(this);
	m_pButtonFFExit->Show();

	for (int i = 0; i < 6; i++)
	{
		auto* pNewButton = new Button(std::to_wstring(i));
		pNewButton->SetBounds(startRowPos.X + i * 40, startRowPos.Y, 30, 30);
		pNewButton->AddActionListener(this);
		pNewButton->Show();
		m_pButtonsFFRooms.push_back(pNewButton);
	}


	// Initialize Rooms
	const auto wallThickness = 30.f;
	m_pRoomQuads.push_back(new Quad(Vector2f(wallThickness, GAME_ENGINE->GetHeight() -
		(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f - wallThickness / 2.f), Vector2f(
			sceneWidth - wallThickness * 2.f,
			(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f - wallThickness / 2.f)));
	m_pRoomQuads.push_back(new Quad(Vector2f(wallThickness, wallThickness), Vector2f(
		(sceneWidth - wallThickness * 2.f) / 4.f - wallThickness / 2.f,
		(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f - wallThickness / 2.f)));
	m_pRoomQuads.push_back(new Quad(Vector2f((sceneWidth - wallThickness * 2.f) / 4.f + wallThickness * 1.5f, wallThickness), Vector2f(
		(sceneWidth - wallThickness * 2.f) / 2.f - wallThickness,
		(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 2.f - wallThickness / 2.f)));
	m_pRoomQuads.push_back(new Quad(Vector2f((sceneWidth - wallThickness * 2.f) / 4.f + wallThickness * 1.5f,
		(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 2.f + wallThickness), Vector2f(
			(sceneWidth - wallThickness * 2.f) / 2.f - wallThickness,
			(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 2.f - wallThickness / 2.f)));

	m_pRoomQuads.push_back(new Quad(Vector2f(sceneWidth -
		(sceneWidth - wallThickness * 2.f) / 4.f - wallThickness / 2.f, wallThickness), Vector2f(
			(sceneWidth - wallThickness * 2.f) / 4.f - wallThickness / 2.f,
			(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f - wallThickness / 2.f)));


	// Initialize agents
	InitializeRegularAgents();
	Initialize3rdPartyAgent();


	// Initialize exit
	m_pExit = new EscapeZone(Vector2f(sceneWidth - wallThickness - 50.f, GAME_ENGINE->GetHeight() - wallThickness - (GAME_ENGINE->GetHeight() - wallThickness * 3.f) / 10.f),
		m_pAgents, Vector2f(50.f, 90.f));


	// Initialize hazards
	m_pHazards.push_back(new Hazard(Vector2f(sceneWidth / 2.f, (GAME_ENGINE->GetHeight() - wallThickness * 3.f) / 5.f + wallThickness), m_pAgents, Vector2f(20.f, 20.f), 200.f));


	// Initialize walls
	//// Outer walls
	m_pWalls.push_back(new Wall(Vector2f(sceneWidth - wallThickness / 6.f * 5.f, GAME_ENGINE->GetHeight() / 2.f), m_pAgents, Vector2f(wallThickness, GAME_ENGINE->GetHeight()), true));
	m_pWalls.push_back(new Wall(Vector2f(wallThickness / 2.f, GAME_ENGINE->GetHeight() / 2.f), m_pAgents, Vector2f(wallThickness, GAME_ENGINE->GetHeight()), true));
	m_pWalls.push_back(new Wall(Vector2f(sceneWidth / 2.f, GAME_ENGINE->GetHeight() - wallThickness / 6.f * 5.f), m_pAgents, Vector2f(sceneWidth - wallThickness, wallThickness), true));
	m_pWalls.push_back(new Wall(Vector2f(sceneWidth / 2.f, wallThickness / 2.f), m_pAgents, Vector2f(sceneWidth - wallThickness, wallThickness), true));
	//// Bottom Horiz Walls
	m_pWalls.push_back(new Wall(Vector2f(wallThickness + (sceneWidth - wallThickness * 4.f) / 5.f / 6.f * 23.f / 2.f + wallThickness + (sceneWidth - wallThickness * 4.f) / 5.f / 2.f,
		wallThickness + (GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f), m_pAgents, Vector2f(sceneWidth - wallThickness * 4.f - (sceneWidth - wallThickness * 4.f) / 5.f / 6.f * 7.f, wallThickness), true));
	m_pWalls.push_back(new Wall(Vector2f(sceneWidth - wallThickness - (sceneWidth - wallThickness * 4.f) / 5.f / 3.f / 2.f - wallThickness / 2.f,
		wallThickness + (GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f), m_pAgents, Vector2f((sceneWidth - wallThickness * 4.f) / 5.f / 2.f, wallThickness), true));
	//// Middle Horiz Wall
	m_pWalls.push_back(new Wall(Vector2f(wallThickness + (sceneWidth - wallThickness * 2.f) / 2.f,
		wallThickness + (GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 2.f), m_pAgents, Vector2f((sceneWidth - wallThickness * 2.f) / 2.f, wallThickness), true));
	//// Left Top Vert Walls
	m_pWalls.push_back(new Wall(Vector2f(wallThickness + (sceneWidth - wallThickness * 2.f) / 4.f,
		wallThickness + (GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 2.f), m_pAgents, Vector2f(wallThickness, ((GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f - wallThickness) / 6.f * 2.f + wallThickness), true));
	m_pWalls.push_back(new Wall(Vector2f(wallThickness + (sceneWidth - wallThickness * 2.f) / 4.f,
		wallThickness + (GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f / 6.f / 2.f), m_pAgents, Vector2f(30.f, (GAME_ENGINE->GetHeight() - wallThickness) / 5.f * 4.f / 6.f), true));
	m_pWalls.push_back(new Wall(Vector2f(wallThickness + (sceneWidth - wallThickness * 2.f) / 4.f,
		wallThickness + (GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f - ((GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 2.f + wallThickness) / 3.f / 2.f), m_pAgents, Vector2f(wallThickness, (GAME_ENGINE->GetHeight() - wallThickness) / 5.f * 4.f / 6.f), true));
	//// Right Top Vert Walls
	m_pWalls.push_back(new Wall(Vector2f(wallThickness + (sceneWidth - wallThickness * 2.f) / 4.f * 3.f,
		wallThickness + (((GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f) - wallThickness) / 6.f / 2.f), m_pAgents, Vector2f(wallThickness, ((GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f - wallThickness) / 6.f), true));
	m_pWalls.push_back(new Wall(Vector2f(wallThickness + (sceneWidth - wallThickness * 2.f) / 4.f * 3.f,
		wallThickness + (((GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f) - wallThickness) / 6.f * 2.f + ((((GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f) - wallThickness) / 6.f * 4.f + wallThickness) / 2.f), m_pAgents, Vector2f(wallThickness, ((GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 4.f - wallThickness) / 6.f * 4.f + wallThickness), true));


	// Initialize FlowFields
	m_pExitFlowField = new FlowField(Vector2f(sceneWidth, GAME_ENGINE->GetHeight()), Vector2f(44, 33), m_pWalls, m_pHazards, m_pExit->GetPosition());
	m_pRoomFlowFields.push_back(new FlowField(Vector2f(sceneWidth, GAME_ENGINE->GetHeight()), Vector2f(44, 33), m_pWalls, m_pHazards, Vector2f(sceneWidth /2.f, GAME_ENGINE->GetHeight() * 0.9f - wallThickness / 2.f)));
	m_pRoomFlowFields.push_back(new FlowField(Vector2f(sceneWidth, GAME_ENGINE->GetHeight()), Vector2f(44, 33), m_pWalls, m_pHazards, Vector2f(sceneWidth / 8.f, (GAME_ENGINE->GetHeight() - wallThickness * 3.f) / 5.f * 2.f + wallThickness)));
	m_pRoomFlowFields.push_back(new FlowField(Vector2f(sceneWidth, GAME_ENGINE->GetHeight()), Vector2f(44, 33), m_pWalls, std::vector<Hazard*>(), Vector2f(sceneWidth / 2.f, (GAME_ENGINE->GetHeight() - wallThickness * 3.f) / 5.f + wallThickness)));
	m_pRoomFlowFields.push_back(new FlowField(Vector2f(sceneWidth, GAME_ENGINE->GetHeight()), Vector2f(44, 33), m_pWalls, m_pHazards, Vector2f(sceneWidth / 2.f, (GAME_ENGINE->GetHeight() - wallThickness * 3.f) / 5.f * 3.f + wallThickness * 2.f)));
	m_pRoomFlowFields.push_back(new FlowField(Vector2f(sceneWidth, GAME_ENGINE->GetHeight()), Vector2f(44, 33), m_pWalls, m_pHazards, Vector2f(sceneWidth / 8.f * 7.f, (GAME_ENGINE->GetHeight() - wallThickness * 3.f) / 5.f * 2.f + wallThickness)));


	Reset();
}

void EmotionContagionSim::End()
{
	delete m_pRegularResetButton;
	m_pRegularResetButton = nullptr;

	delete m_pNoAuthorityResetButton;
	m_pNoAuthorityResetButton = nullptr;

	delete m_pButtonFFExit;
	m_pButtonFFExit = nullptr;

	for (auto* pButton : m_pButtonsFFRooms)
	{
		delete pButton;
		pButton = nullptr;
	}
	m_pButtonsFFRooms.clear();

	for (auto* pAgent : m_pAgents)
	{
		delete pAgent;
		pAgent = nullptr;
	}
	m_pAgents.clear();

	delete m_pExit;
	m_pExit = nullptr;

	for (auto* pHazard : m_pHazards)
	{
		delete pHazard;
		pHazard = nullptr;
	}
	m_pHazards.clear();

	for (auto* pWall : m_pWalls)
	{
		delete pWall;
		pWall = nullptr;
	}
	m_pWalls.clear();

	for (auto* pFlowField : m_pRoomFlowFields)
	{
		delete pFlowField;
		pFlowField = nullptr;
	}
	m_pRoomFlowFields.clear();

	delete m_pExitFlowField;
	m_pExitFlowField = nullptr;

	for (auto* pRoomQuad : m_pRoomQuads)
	{
		delete pRoomQuad;
		pRoomQuad = nullptr;
	}
	m_pRoomQuads.clear();
}

void EmotionContagionSim::MouseButtonAction(bool isLeft, bool isDown, int x, int y, WPARAM wParam)
{	
	// Insert the code that needs to be executed when the project registers a mouse button action

	if (isLeft == true && isDown == true) // Left Mouse Click
	{	
		auto agentClicked = false;
		for (auto* pAgent : m_pAgents)
		{
			if (pAgent != m_pPickedAgent && !pAgent->GetHasEscaped())
			{
				if (Vector2f(float(x), float(y)).IsWithinCircleArea(pAgent->GetPosition(), pAgent->GetAgentDiameter() / 2.f))
				{
					agentClicked = true;
					m_pPickedAgent = pAgent;
					break;
				}
			}
		}

		if (!agentClicked) m_pPickedAgent = nullptr;
	}
	else if (isLeft == false && isDown == true) // Right Mouse Click
	{
		m_TimeMultiplier++;
		if (m_TimeMultiplier > 3) m_TimeMultiplier = 1;
	}
}

void EmotionContagionSim::MouseWheelAction(int x, int y, int distance, WPARAM wParam)
{	
	// Insert the code that needs to be executed when the project registers a mouse wheel action
}

void EmotionContagionSim::MouseMove(int x, int y, WPARAM wParam)
{	
	// Insert the code that needs to be executed when the mouse pointer moves across the project window

	/* Example:
	if ( x > 261 && x < 261 + 117 ) // check if mouse position is within x coordinates of choice
	{
		if ( y > 182 && y < 182 + 33 ) // check if mouse position also is within y coordinates of choice
		{
			GAME_ENGINE->MessageBox(_T("Da mouse wuz here."));
		}
	}
	*/
}

void EmotionContagionSim::CheckKeyboard()
{	
	// Here you can check if a key of choice is held down
	// Is executed once per frame if the Project Loop is running 

	/* Example:
	if (GAME_ENGINE->IsKeyDown(_T('K'))) xIcon -= xSpeed;
	if (GAME_ENGINE->IsKeyDown(_T('L'))) yIcon += xSpeed;
	if (GAME_ENGINE->IsKeyDown(_T('M'))) xIcon += xSpeed;
	if (GAME_ENGINE->IsKeyDown(_T('O'))) yIcon -= ySpeed;
	*/
}

void EmotionContagionSim::KeyPressed(TCHAR cKey)
{	
	// DO NOT FORGET to use SetKeyList() !!

	// Insert the code that needs to be executed when a key of choice is pressed
	// Is executed as soon as the key is released
	// You first need to specify the keys that the research engine needs to watch by using the SetKeyList() method

	/* Example:
	switch (cKey)
	{
	case _T('K'): case VK_LEFT:
		GAME_ENGINE->MessageBox(_T("Moving left."));
		break;
	case _T('L'): case VK_DOWN:
		GAME_ENGINE->MessageBox(_T("Moving down."));
		break;
	case _T('M'): case VK_RIGHT:
		GAME_ENGINE->MessageBox(_T("Moving right."));
		break;
	case _T('O'): case VK_UP:
		GAME_ENGINE->MessageBox(_T("Moving up."));
		break;
	case VK_ESCAPE:
		GAME_ENGINE->MessageBox(_T("Escape menu."));
	}
	*/
}

void EmotionContagionSim::Paint(RECT rect)
{
	GAME_ENGINE->DrawSolidBackground(RGB(10, 10, 10));

	if (m_pVisibleFF != 0)
	{
		GAME_ENGINE->SetColor(RGB(0, 0, 100));
		GAME_ENGINE->FillRect(m_pRoomQuads[m_pVisibleFF - 1]->BottomLeft.X, m_pRoomQuads[m_pVisibleFF - 1]->BottomLeft.Y,
			m_pRoomQuads[m_pVisibleFF - 1]->Size.X, m_pRoomQuads[m_pVisibleFF - 1]->Size.Y);
	}

	for (auto* pAgent : m_pAgents) pAgent->DrawBackground();
	m_pExit->Draw(bool(m_pVisibleFF == 0));
	for (auto* pHazard : m_pHazards) pHazard->Draw();
	for (auto* pWall : m_pWalls) pWall->Draw();


	if (m_pVisibleFF == 0) m_pExitFlowField->Draw();
	else m_pRoomFlowFields[m_pVisibleFF - 1]->Draw();

	for (auto* pAgent : m_pAgents) pAgent->DrawForward(pAgent == m_pPickedAgent);

	if (m_pPickedAgent == nullptr)
	{
		GAME_ENGINE->SetColor(RGB(255, 255, 255));
		GAME_ENGINE->DrawString(L"---> Click on an agent! <---", GAME_ENGINE->GetWidth() - m_SideWindowWidth / 2 - 85, GAME_ENGINE->GetHeight() / 2 - 5);
	}
}

void EmotionContagionSim::Tick()
{
	//GAME_ENGINE->GetFrameDelay();
	const float elapsedSec = 1.f / 50.f;


	if (!m_RunTests)
	{
		// Insert non-paint code that needs to be executed each tick 
		for (auto* pHazard : m_pHazards) pHazard->Update(elapsedSec * float(m_TimeMultiplier));
		for (auto* pAgent : m_pAgents) pAgent->Update(elapsedSec * float(m_TimeMultiplier));
		for (auto* pWall : m_pWalls) pWall->Update();

		m_pExit->Update();
		return;
	}



	if (m_DoneTestsNumber < m_DesiredTestsNumber)
	{
		m_CurrentTestTime += elapsedSec * 10.f;

		// Insert non-paint code that needs to be executed each tick 
		for (auto* pHazard : m_pHazards) pHazard->Update(elapsedSec * 10.f);
		for (auto* pAgent : m_pAgents) pAgent->Update(elapsedSec * 10.f);
		for (auto* pWall : m_pWalls) pWall->Update();

		m_pExit->Update();

		auto testDone = true;
		for (auto* pAgent : m_pAgents)
		{
			if (!pAgent->GetHasEscaped())
			{
				testDone = false;
				break;
			}
		}

		if (testDone)
		{
			m_DoneTestsNumber++;

			ofstream myfile;
			myfile.open("tests.txt", std::ios_base::app);
			myfile << "Test #" + std::to_string(m_DoneTestsNumber) + " - " + std::to_string(m_CurrentTestTime) + "\n";
			myfile.close();

			m_CurrentTestTime = 0.0f;
			ResetWithoutAuthority();
		}
	}
}

void EmotionContagionSim::CallAction(Caller* callerPtr)
{
	// Insert the code that needs to be executed when a Caller has to perform an action
	if (callerPtr == m_pRegularResetButton) Reset();
	else if(callerPtr == m_pNoAuthorityResetButton) ResetWithoutAuthority();
	else if (callerPtr == m_pButtonFFExit) m_pVisibleFF = 0;
	else
	{
		for (int i = 0; i < m_pButtonsFFRooms.size(); i++)
		{
			if (callerPtr == m_pButtonsFFRooms[i])
			{
				m_pVisibleFF = i;
				break;
			}
		}
	}
}

void EmotionContagionSim::InitializeRegularAgents()
{
	// Spawn regular agents randomly within the rooms
	const auto minWallDist = 15.f;
	for (int i = 0; i < 100; i++)
	{
		auto finalPosFound = false;
		Vector2f finalPos;

		// Get a random room to spawn in
		auto randomRoomIdx = rand() % 5;
		//if (randomRoomIdx == 2) randomRoomIdx = 4;
		auto* pRandRoomQuad = m_pRoomQuads[randomRoomIdx];

		// Get a random position within that room
		auto randomPos = Vector2f((pRandRoomQuad->BottomLeft.X + minWallDist) + (pRandRoomQuad->Size.X - minWallDist * 2.f) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)),
			(pRandRoomQuad->BottomLeft.Y + minWallDist) + (pRandRoomQuad->Size.Y - minWallDist * 2.f) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)));
		
		// If that position overlaps with a hazard, find another one
		auto posInvalid = true;
		while (posInvalid)
		{
			auto hitAnyObstacle = false;
			for (auto* pHazard : m_pHazards)
			{
				if (randomPos.IsWithinCircleArea(pHazard->GetPosition(), pHazard->GetEffectRange() * 0.7f))
				{
					hitAnyObstacle = true;
					break;
				}
			}

			if (hitAnyObstacle)
			{
				randomPos = Vector2f((pRandRoomQuad->BottomLeft.X + minWallDist) + (pRandRoomQuad->Size.X - minWallDist * 2.f) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)),
					(pRandRoomQuad->BottomLeft.Y + minWallDist) + (pRandRoomQuad->Size.Y - minWallDist * 2.f) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)));
			}
			else posInvalid = false;
		}

		auto agentProfile = OCEANProfile((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f + 0.1f,
			(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f + 0.1f,
			(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f + 0.1f,
			(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f + 0.1f,
			(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f + 0.1f);

		m_pAgents.push_back(new RegularAgent(randomPos, Vector2f(1.f, 0.f), agentProfile, m_pExitFlowField, m_pRoomFlowFields, m_pRoomQuads, &m_pWalls, &m_pAgents));
	}
}

void EmotionContagionSim::Initialize3rdPartyAgent()
{
	// Spawn the 3rd party agent in the middle room (room 3, counting from 0)
	const auto wallThickness = 60;
	const auto sceneWidth = (GAME_ENGINE->GetWidth() - m_SideWindowWidth - wallThickness * 2);

	const auto middleRoomQuad = Quad(Vector2f((sceneWidth - wallThickness * 2.f) / 4.f + wallThickness * 1.5f,
		(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 2.f + wallThickness), Vector2f(
			(sceneWidth - wallThickness * 2.f) / 2.f - wallThickness,
			(GAME_ENGINE->GetHeight() - wallThickness * 2.f) / 5.f * 2.f - wallThickness / 2.f));

	m_pAgents.push_back(new ThirdPartyAgent(middleRoomQuad.BottomLeft + middleRoomQuad.Size / 2.f, Vector2f(1.f, 0.f), m_pExitFlowField, m_pRoomFlowFields, m_pRoomQuads, &m_pWalls, &m_pAgents));
}

void EmotionContagionSim::Reset()
{
	m_pPickedAgent = nullptr;

	for (auto* pAgent : m_pAgents)
	{
		delete pAgent;
		pAgent = nullptr;
	}
	m_pAgents.clear();

	InitializeRegularAgents();
	Initialize3rdPartyAgent();

	m_pExit->ResetAgents(m_pAgents);
	for (auto* pHazard : m_pHazards) pHazard->ResetAgents(m_pAgents);
	for (auto* pWall : m_pWalls) pWall->ResetAgents(m_pAgents);
}

void EmotionContagionSim::ResetWithoutAuthority()
{
	m_pPickedAgent = nullptr;

	for (auto* pAgent : m_pAgents)
	{
		delete pAgent;
		pAgent = nullptr;
	}
	m_pAgents.clear();

	InitializeRegularAgents();

	m_pExit->ResetAgents(m_pAgents);
	for (auto* pHazard : m_pHazards) pHazard->ResetAgents(m_pAgents);
	for (auto* pWall : m_pWalls) pWall->ResetAgents(m_pAgents);
}




