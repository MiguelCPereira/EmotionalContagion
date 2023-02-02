#include "RegularAgent.h"
#include "GameEngine.h"
#include "SteeringBehaviors.h"
#include "Hazard.h"
#include "Wall.h"
#include "FlowField.h"

RegularAgent::RegularAgent(Vector2f initialPos, Vector2f initialDir, OCEANProfile emotionalProfile, FlowField* pExitFlowField,
	std::vector<FlowField*> pRoomsFlowFields, std::vector<Quad*> pRoomsQuads, std::vector<Wall*>* pWalls, std::vector<RegularAgent*>* pAgents)
	: m_HasEscaped()
	, m_pExitFlowField(pExitFlowField)
	, m_pRoomsFlowFields(pRoomsFlowFields)
	, m_pRoomsQuads(pRoomsQuads)
	, m_pWalls(pWalls)
	, m_pAgents(pAgents)

	, m_Position(initialPos)
	, m_PositionLastFrame(initialPos)
	, m_MovementDirection(initialDir)
	, m_FacingDirection(initialDir)
	, m_TargetPos(initialPos)
	, m_CurrentSpeed(40.f)
	, m_RegularSpeed(40.f)
	, m_SprintSpeed(100.f)
	, m_SlowSpeed(30.f)
	, m_AgentDiameter(20.f)
	, m_EmotionalProfile(emotionalProfile)
	, m_OtherAgentAwarenessRadius(50.f)

	, m_CurrentRoomIdx()
	, m_LastRoomBeforeCurrentIdx(-1)
	, m_CurrentlySeekingNewRoom()
	, m_CurrentlySeekedRoomIdx()
	, m_RoomExploringMinMaxTime(1.f + (1.f - emotionalProfile.Openess) * 6.f, 6.f + (1.f - emotionalProfile.Openess) * 6.f)
	, m_RoomExploringCounter()
	, m_RoomMemoryTime(5.f + emotionalProfile.Conscientiousness * 10.f)

	, m_CalmSpreadRate(0.1f)
	, m_FrightSpreadRate(0.9f)
	, m_PanicSpreadRate(0.75f)
	, m_CalmAttenuationRate(1.f / 3.f)
	, m_FrightAttenuationRate(1.f / 3.f)
	, m_PanicAttenuationRate(0.3125f)
	, m_MaxEmotionalInfectionDist(150.f)

	, m_PercievedCalm()
	, m_PercievedFright()
	, m_PercievedPanic()

	, m_CurrentEmotion(Emotion::Calm)

	, m_OnEmotionalCooldown()
	, m_EmotionalCooldownTime(1.f)
	, m_EmotionalCooldownCounter()
{
	m_CurrentRoomExploringTime = m_RoomExploringMinMaxTime.X + (m_RoomExploringMinMaxTime.Y - m_RoomExploringMinMaxTime.X) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
	for (int i = 0; i < m_pRoomsQuads.size(); i++)
	{
		if (m_pRoomsQuads[i]->IsOverlappingCircle(m_Position, m_AgentDiameter / 2.f))
		{
			m_Rooms.push_back(std::make_pair(true, 0.f));
			m_CurrentRoomIdx = i;
		}
		else
		{
			m_Rooms.push_back(std::make_pair(false, 0.f));
		}
	}

	m_pSeekBehavior = new Seek();
	m_pFleeBehavior = new Flee();
	m_pWanderBehavior = new Wander();
}

RegularAgent::~RegularAgent()
{
	delete m_pSeekBehavior;
	m_pSeekBehavior = nullptr;
	delete m_pFleeBehavior;
	m_pFleeBehavior = nullptr;
	delete m_pWanderBehavior;
	m_pWanderBehavior = nullptr;
}

void RegularAgent::DrawBackground()
{
	if (m_HasEscaped) return;

	// Draw the other agent awareness radius
	GAME_ENGINE->SetColor(RGB(50, 50, 50));
	GAME_ENGINE->FillOval(int(m_Position.X - m_OtherAgentAwarenessRadius / 2.f), int(m_Position.Y - m_OtherAgentAwarenessRadius / 2.f), int(m_OtherAgentAwarenessRadius), int(m_OtherAgentAwarenessRadius));
}

void RegularAgent::DrawForward(bool isPicked)
{
	if (isPicked) DrawHUD();

	if (m_HasEscaped) return;

	// Draw an outline if picked
	if (isPicked)
	{
		const auto outlineThickness = 4.f;
		GAME_ENGINE->SetColor(RGB(255, 255, 255));
		GAME_ENGINE->FillOval(int(m_Position.X - (m_AgentDiameter + outlineThickness * 2.f) / 2.f), int(m_Position.Y - (m_AgentDiameter + outlineThickness * 2.f) / 2.f), int(m_AgentDiameter + outlineThickness * 2.f), int(m_AgentDiameter + outlineThickness * 2.f));
	}

	// Draw the movement direction
	GAME_ENGINE->SetColor(RGB(255, 255, 0));
	GAME_ENGINE->DrawLine(int(m_Position.X), int(m_Position.Y),
		int(m_Position.X + m_MovementDirection.X * m_AgentDiameter),
		int(m_Position.Y + m_MovementDirection.Y * m_AgentDiameter));

	// Draw the agent body
	//GAME_ENGINE->SetColor(RGB(int(m_pCurrentBehavior->GetColor().X), int(m_pCurrentBehavior->GetColor().Y), int(m_pCurrentBehavior->GetColor().Z)));
	switch (m_CurrentEmotion)
	{
	case Emotion::Calm:
		if(m_CurrentlySeekingNewRoom) GAME_ENGINE->SetColor(RGB(255, 228, 181));
		else GAME_ENGINE->SetColor(RGB(204, 204, 255));
		break;
	case Emotion::Fright:
		GAME_ENGINE->SetColor(RGB(37, 150, 118));
		break;
	case Emotion::Panic:
		GAME_ENGINE->SetColor(RGB(255, 99, 71));
		break;
	}
	GAME_ENGINE->FillOval(int(m_Position.X - m_AgentDiameter / 2.f), int(m_Position.Y - m_AgentDiameter / 2.f), int(m_AgentDiameter), int(m_AgentDiameter));

	// Draw the facing direction
	GAME_ENGINE->SetColor(RGB(0, 0, 0));
	GAME_ENGINE->DrawLine(int(m_Position.X), int(m_Position.Y),
		int(m_Position.X + m_FacingDirection.X * m_AgentDiameter / 2.f),
		int(m_Position.Y + m_FacingDirection.Y * m_AgentDiameter / 2.f));
}

void RegularAgent::DrawHUD()
{
	const auto sideWindowWidth = 300;
	const Vector2f hudAgentCenter = Vector2f(GAME_ENGINE->GetWidth() - sideWindowWidth / 2.f - 7.f, GAME_ENGINE->GetHeight() / 7.f - 25);
	const float hudAgentDiameter = 70.f;

	// Draw Agent Background
	GAME_ENGINE->SetColor(RGB(100, 100, 100));
	GAME_ENGINE->FillOval(int(hudAgentCenter.X - hudAgentDiameter), int(hudAgentCenter.Y - hudAgentDiameter), int(hudAgentDiameter * 2.f), int(hudAgentDiameter * 2.f));


	// Draw Agent
	if (m_HasEscaped)
	{
		GAME_ENGINE->SetColor(RGB(180, 180, 180));
		GAME_ENGINE->FillOval(int(hudAgentCenter.X - hudAgentDiameter / 2.f), int(hudAgentCenter.Y - hudAgentDiameter / 2.f), int(hudAgentDiameter), int(hudAgentDiameter));
	}
	else
	{
		GAME_ENGINE->SetColor(RGB(255, 255, 0));
		GAME_ENGINE->DrawLine(int(hudAgentCenter.X), int(hudAgentCenter.Y),
			int(hudAgentCenter.X + m_MovementDirection.X * hudAgentDiameter),
			int(hudAgentCenter.Y + m_MovementDirection.Y * hudAgentDiameter));
		switch (m_CurrentEmotion)
		{
		case Emotion::Calm:
			if (m_CurrentlySeekingNewRoom) GAME_ENGINE->SetColor(RGB(255, 228, 181));
			else GAME_ENGINE->SetColor(RGB(204, 204, 255));
			break;
		case Emotion::Fright:
			GAME_ENGINE->SetColor(RGB(37, 150, 118));
			break;
		case Emotion::Panic:
			GAME_ENGINE->SetColor(RGB(255, 99, 71));
			break;
		}
		GAME_ENGINE->FillOval(int(hudAgentCenter.X - hudAgentDiameter / 2.f), int(hudAgentCenter.Y - hudAgentDiameter / 2.f), int(hudAgentDiameter), int(hudAgentDiameter));
		GAME_ENGINE->SetColor(RGB(0, 0, 0));
		GAME_ENGINE->DrawLine(int(hudAgentCenter.X), int(hudAgentCenter.Y),
			int(hudAgentCenter.X + m_FacingDirection.X * hudAgentDiameter / 2.f),
			int(hudAgentCenter.Y + m_FacingDirection.Y * hudAgentDiameter / 2.f));
	}


	// Write out current emotion
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	if (m_HasEscaped)
	{
		GAME_ENGINE->DrawString(L"Has Escaped", int(hudAgentCenter.X - 42.f), int(hudAgentCenter.Y + hudAgentDiameter + 25.f));
	}
	else
	{
		switch (m_CurrentEmotion)
		{
		case Emotion::Calm:
			GAME_ENGINE->DrawString(L"Current Emotion: Calm", int(hudAgentCenter.X - 74.f), int(hudAgentCenter.Y + hudAgentDiameter + 15.f));
			if (!m_CurrentlySeekingNewRoom) GAME_ENGINE->DrawString(L"Current Action: Wander", int(hudAgentCenter.X - 75.f), int(hudAgentCenter.Y + hudAgentDiameter + 40.f));
			else GAME_ENGINE->DrawString(L"Current Action: Change Room", int(hudAgentCenter.X - 95.f), int(hudAgentCenter.Y + hudAgentDiameter + 35.f));
			break;
		case Emotion::Fright:
			GAME_ENGINE->DrawString(L"Current Emotion: Afraid", int(hudAgentCenter.X - 76.f), int(hudAgentCenter.Y + hudAgentDiameter + 15.f));
			GAME_ENGINE->DrawString(L"Current Action: Evacuate", int(hudAgentCenter.X - 79.f), int(hudAgentCenter.Y + hudAgentDiameter + 35.f));
			break;
		case Emotion::Panic:
			GAME_ENGINE->DrawString(L"Current Emotion: Panic", int(hudAgentCenter.X - 74.f), int(hudAgentCenter.Y + hudAgentDiameter + 15.f));
			GAME_ENGINE->DrawString(L"Current Action: Run Aimlessly", int(hudAgentCenter.X - 98.f), int(hudAgentCenter.Y + hudAgentDiameter + 35.f));
			break;
		}
	}

	// Draw Line 1
	const auto lineGap = 20;
	const auto line1PosY = int(hudAgentCenter.Y + hudAgentDiameter + 50.f) + 5 + lineGap;
	GAME_ENGINE->SetColor(RGB(220, 220, 220));
	GAME_ENGINE->DrawLine(GAME_ENGINE->GetWidth() - sideWindowWidth + 30 - 7, line1PosY, GAME_ENGINE->GetWidth() - 30 - 7, line1PosY);

	// Draw OCEAN bars
	const auto oceanBarWidth = 200.f;
	const auto oceanBarHeight = 15.f;
	const auto gapTitleToBar = 20;
	const auto gapToNextTitle = 4;
	const auto fullBarGap = gapTitleToBar + oceanBarHeight + gapToNextTitle;
	const auto barMargin = 3;

	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(L"Openess", int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap);
	GAME_ENGINE->SetColor(RGB(100, 50, 50));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth/2.f), line1PosY + lineGap + gapTitleToBar, int(oceanBarWidth), int(oceanBarHeight), 10);

	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(L"Conscientiousness", int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap + fullBarGap);
	GAME_ENGINE->SetColor(RGB(50, 70, 90));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap + fullBarGap + gapTitleToBar, int(oceanBarWidth), int(oceanBarHeight), 10);
	
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(L"Extraversion", int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap + fullBarGap * 2);
	GAME_ENGINE->SetColor(RGB(50, 95, 55));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap + fullBarGap * 2 + gapTitleToBar, int(oceanBarWidth), int(oceanBarHeight), 10);
	
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(L"Agreeableness", int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap + fullBarGap * 3);
	GAME_ENGINE->SetColor(RGB(90, 90, 30));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap + fullBarGap * 3 + gapTitleToBar, int(oceanBarWidth), int(oceanBarHeight), 10);
	
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(L"Neuroticism", int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap + fullBarGap * 4);
	GAME_ENGINE->SetColor(RGB(90, 50, 70));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f), line1PosY + lineGap + fullBarGap * 4 + gapTitleToBar, int(oceanBarWidth), int(oceanBarHeight), 10);

	GAME_ENGINE->SetColor(RGB(0, 0, 0));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + gapTitleToBar + barMargin, int(oceanBarWidth - barMargin * 2), int(oceanBarHeight - barMargin * 2), 10);
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + fullBarGap + gapTitleToBar + barMargin, int(oceanBarWidth - barMargin * 2), int(oceanBarHeight - barMargin * 2), 10);
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + fullBarGap * 2 + gapTitleToBar + barMargin, int(oceanBarWidth - barMargin * 2), int(oceanBarHeight - barMargin * 2), 10);
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + fullBarGap * 3 + gapTitleToBar + barMargin, int(oceanBarWidth - barMargin * 2), int(oceanBarHeight - barMargin * 2), 10);
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + fullBarGap * 4 + gapTitleToBar + barMargin, int(oceanBarWidth - barMargin * 2), int(oceanBarHeight - barMargin * 2), 10);
	
	GAME_ENGINE->SetColor(RGB(200, 50, 50));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + gapTitleToBar + barMargin, int((oceanBarWidth - float(barMargin) * 2.f) * m_EmotionalProfile.Openess), int(oceanBarHeight - barMargin * 2), 10);
	GAME_ENGINE->SetColor(RGB(50, 140, 180));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + fullBarGap + gapTitleToBar + barMargin, int((oceanBarWidth - float(barMargin) * 2.f) * m_EmotionalProfile.Conscientiousness), int(oceanBarHeight - barMargin * 2), 10);
	GAME_ENGINE->SetColor(RGB(50, 185, 110));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + fullBarGap * 2 + gapTitleToBar + barMargin, int((oceanBarWidth - float(barMargin) * 2.f) * m_EmotionalProfile.Extraversion), int(oceanBarHeight - barMargin * 2), 10);
	GAME_ENGINE->SetColor(RGB(170, 170, 30));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + fullBarGap * 3 + gapTitleToBar + barMargin, int((oceanBarWidth - float(barMargin) * 2.f) * m_EmotionalProfile.Agreeableness), int(oceanBarHeight - barMargin * 2), 10);
	GAME_ENGINE->SetColor(RGB(180, 50, 140));
	GAME_ENGINE->FillRoundRect(int(hudAgentCenter.X - oceanBarWidth / 2.f + barMargin), line1PosY + lineGap + fullBarGap * 4 + gapTitleToBar + barMargin, int((oceanBarWidth - float(barMargin) * 2.f) * m_EmotionalProfile.Neuroticism), int(oceanBarHeight - barMargin * 2), 10);


	// Draw Line 2
	const auto line2PosY = line1PosY + lineGap + fullBarGap * 4 + gapTitleToBar + oceanBarHeight + lineGap;
	GAME_ENGINE->SetColor(RGB(220, 220, 220));
	GAME_ENGINE->DrawLine(GAME_ENGINE->GetWidth() - sideWindowWidth + 30 - 7, line2PosY, GAME_ENGINE->GetWidth() - 30 - 7, line2PosY);


	// Write emotions accumulation
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(L"Accumulated Emotions", int(hudAgentCenter.X - 76.f), line2PosY + lineGap);
	GAME_ENGINE->SetColor(RGB(180, 180, 180));
	auto calmValueText = std::to_wstring(int(m_PercievedCalm * 100.f));
	auto originalDigitsAmount = calmValueText.size();
	if (originalDigitsAmount > 0 && originalDigitsAmount < 3){ for (int i = 0; i < 3 - originalDigitsAmount; i++) calmValueText = L"0" + calmValueText; }
	GAME_ENGINE->DrawString(L"Calm: " + calmValueText + std::wstring(L" / 100"), int(hudAgentCenter.X - 55.f), line2PosY + lineGap + 30);
	auto afraidValueText = std::to_wstring(int(m_PercievedFright * 100.f));
	originalDigitsAmount = afraidValueText.size();
	if (originalDigitsAmount > 0 && originalDigitsAmount < 3) { for (int i = 0; i < 3 - originalDigitsAmount; i++) afraidValueText = L"0" + afraidValueText; }
	GAME_ENGINE->DrawString(L"Afraid: " + afraidValueText + std::wstring(L" / 100"), int(hudAgentCenter.X - 56.f), line2PosY + lineGap + 50);
	auto panicValueText = std::to_wstring(int(m_PercievedPanic * 100.f));
	originalDigitsAmount = panicValueText.size();
	if (originalDigitsAmount > 0 && originalDigitsAmount < 3) { for (int i = 0; i < 3 - originalDigitsAmount; i++) panicValueText = L"0" + panicValueText; }
	GAME_ENGINE->DrawString(L"Panic: " + panicValueText + std::wstring(L" / 100"), int(hudAgentCenter.X - 55.f), line2PosY + lineGap + 70);


	// Draw Line 3
	const auto line3PosY = line2PosY + lineGap * 2 + 70 + 15;
	GAME_ENGINE->SetColor(RGB(220, 220, 220));
	GAME_ENGINE->DrawLine(GAME_ENGINE->GetWidth() - sideWindowWidth + 30 - 7, line3PosY, GAME_ENGINE->GetWidth() - 30 - 7, line3PosY);


	// Rooms Memorized Title
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(L"Rooms Memorized", int(hudAgentCenter.X - 64.f), line3PosY + lineGap);


	// Rooms
	const auto mapSizeY = GAME_ENGINE->GetHeight() / 10;
	const auto mapSizeX = (GAME_ENGINE->GetWidth() - sideWindowWidth) / 10;
	const auto mapTop = line3PosY + lineGap + 35;
	const auto mapLeft = GAME_ENGINE->GetWidth() - sideWindowWidth / 2 - mapSizeX / 2 - 7;
	std::wstring roomTime;
	if (m_Rooms[0].first)
	{
		GAME_ENGINE->SetColor(RGB(30, 150, 30));
		GAME_ENGINE->FillRect(mapLeft, mapTop + mapSizeY / 5 * 4, mapSizeX, mapSizeY / 5);
		roomTime = std::to_wstring(int(m_Rooms[0].second));
		GAME_ENGINE->SetColor(RGB(0, 0, 0));
		GAME_ENGINE->DrawString(roomTime, mapLeft + 2, mapTop + mapSizeY / 5 * 4);
	}
	if (m_Rooms[1].first)
	{
		GAME_ENGINE->SetColor(RGB(30, 150, 30));
		GAME_ENGINE->FillRect(mapLeft, mapTop, mapSizeX / 4, mapSizeY / 5 * 4);
		roomTime = std::to_wstring(int(m_Rooms[1].second));
		GAME_ENGINE->SetColor(RGB(0, 0, 0));
		GAME_ENGINE->DrawString(roomTime, mapLeft + 2, mapTop);
	}
	if (m_Rooms[2].first)
	{
		GAME_ENGINE->SetColor(RGB(30, 150, 30));
		GAME_ENGINE->FillRect(mapLeft + mapSizeX / 4, mapTop, mapSizeX / 2, mapSizeY / 5 * 2);
		roomTime = std::to_wstring(int(m_Rooms[2].second));
		GAME_ENGINE->SetColor(RGB(0, 0, 0));
		GAME_ENGINE->DrawString(roomTime, mapLeft + mapSizeX / 4 + 2, mapTop);
	}
	if (m_Rooms[3].first)
	{
		GAME_ENGINE->SetColor(RGB(30, 150, 30));
		GAME_ENGINE->FillRect(mapLeft + mapSizeX / 4, mapTop + mapSizeY / 5 * 2, mapSizeX / 2, mapSizeY / 5 * 2);
		roomTime = std::to_wstring(int(m_Rooms[3].second));
		GAME_ENGINE->SetColor(RGB(0, 0, 0));
		GAME_ENGINE->DrawString(roomTime, mapLeft + mapSizeX / 4 + 2, mapTop + mapSizeY / 5 * 2);
	}
	if (m_Rooms[4].first)
	{
		GAME_ENGINE->SetColor(RGB(30, 150, 30));
		GAME_ENGINE->FillRect(mapLeft + mapSizeX - mapSizeX / 4 - 1, mapTop, mapSizeX / 4, mapSizeY / 5 * 4);
		roomTime = std::to_wstring(int(m_Rooms[4].second));
		GAME_ENGINE->SetColor(RGB(0, 0, 0));
		GAME_ENGINE->DrawString(roomTime, mapLeft + mapSizeX - mapSizeX / 4 - 1 + 2, mapTop);
	}

	// Map Outlines
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawRect(mapLeft, mapTop, mapSizeX, mapSizeY);
	//// Horiz Bottom Left
	GAME_ENGINE->DrawLine(mapLeft + mapSizeX / 8, mapTop + mapSizeY / 5 * 4, mapLeft + mapSizeX / 4 * 3 + mapSizeX / 4 / 3, mapTop + mapSizeY / 5 * 4);
	//// Horiz Bottom Right
	GAME_ENGINE->DrawLine(mapLeft + mapSizeX / 4 * 3 + mapSizeX / 4 / 3 * 2, mapTop + mapSizeY / 5 * 4, mapLeft + mapSizeX, mapTop + mapSizeY / 5 * 4);
	//// Horiz Middle
	GAME_ENGINE->DrawLine(mapLeft + mapSizeX / 4, mapTop + mapSizeY / 5 * 2, mapLeft + mapSizeX / 4 * 3, mapTop + mapSizeY / 5 * 2);
	//// Vert Left Top
	GAME_ENGINE->DrawLine(mapLeft + mapSizeX / 4, mapTop, mapLeft + mapSizeX / 4, mapTop + mapSizeY / 5 * 2 / 3);
	//// Vert Right Top
	GAME_ENGINE->DrawLine(mapLeft + mapSizeX / 4 * 3, mapTop, mapLeft + mapSizeX / 4 * 3, mapTop + mapSizeY / 5 * 2 / 3);
	//// Vert Left Middle
	GAME_ENGINE->DrawLine(mapLeft + mapSizeX / 4, mapTop + mapSizeY / 5 * 2 / 3 * 2, mapLeft + mapSizeX / 4, mapTop + mapSizeY / 5 * 2 / 3 * 4);
	//// Vert Right Bottom
	GAME_ENGINE->DrawLine(mapLeft + mapSizeX / 4 * 3, mapTop + mapSizeY / 5 * 2 / 3 * 2, mapLeft + mapSizeX / 4 * 3, mapTop + mapSizeY / 5 * 4);
	//// Vert Left Bottom
	GAME_ENGINE->DrawLine(mapLeft + mapSizeX / 4, mapTop + mapSizeY / 5 * 2 / 3 * 5, mapLeft + mapSizeX / 4, mapTop + mapSizeY / 5 * 4);


	// Write out the max memory time
	GAME_ENGINE->SetColor(RGB(180, 180, 180));
	auto memoryValueText = std::to_wstring(int(m_RoomMemoryTime));
	auto memoryDigitsAmount = memoryValueText.size();
	if (memoryDigitsAmount > 0 && memoryDigitsAmount < 2) { for (int i = 0; i < 2 - memoryDigitsAmount; i++) memoryValueText = L"0" + memoryValueText; }
	GAME_ENGINE->DrawString(L"Max Memory Time: " + memoryValueText, GAME_ENGINE->GetWidth() - sideWindowWidth / 2 - 82, mapTop + mapSizeY + 10);
}

void RegularAgent::Update(float elapsedSec)
{
	if (m_HasEscaped) return;

	// Get infected by other agents emotionally
	GetEmotionallyInfected(elapsedSec);

	// Progressively forget about past rooms
	for (int i = 0; i < m_Rooms.size(); i++)
	{
		if (m_CurrentRoomIdx != i && m_Rooms[i].first)
		{
			m_Rooms[i].second += elapsedSec;
			if (m_Rooms[i].second >= m_RoomMemoryTime)
			{
				m_Rooms[i].first = false;
				m_Rooms[i].second = 0.f;
			}
		}
	}

	// Update current room
	auto currentRoomChanged = false;
	if (!m_pRoomsQuads[m_CurrentRoomIdx]->IsOverlappingCircle(m_Position, m_AgentDiameter / 2.f))
	{
		for (int i = 0; i < m_pRoomsQuads.size(); i++)
		{
			if (m_pRoomsQuads[i]->IsOverlappingCircle(m_Position, m_AgentDiameter / 2.f))
			{
				m_Rooms[i].first = true;
				m_Rooms[i].second = 0.f;
				m_LastRoomBeforeCurrentIdx = m_CurrentRoomIdx;
				m_CurrentRoomIdx = i;
				currentRoomChanged = true;
				break;
			}
		}
	}


	// Calculate the desired steering, according to active emotion
	auto steeringOutput = SteeringOutput();
	auto randomFactorSteer = SteeringOutput();
	auto fleeFactorSteer = SteeringOutput();
	switch (m_CurrentEmotion)
	{
	case Emotion::Calm:

		if (m_CurrentlySeekingNewRoom)
		{
			// If a new room is reached, stop seeking a new room
			if (currentRoomChanged)
			{
				m_CurrentRoomExploringTime = m_RoomExploringMinMaxTime.X + (m_RoomExploringMinMaxTime.Y - m_RoomExploringMinMaxTime.X) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
				m_CurrentlySeekingNewRoom = false;
			}

			// Follow the flow-field to the target room
			steeringOutput = m_pSeekBehavior->CalculateSteering(this, m_pRoomsFlowFields[m_CurrentlySeekedRoomIdx]->GetNextCellPosition(m_Position), elapsedSec);
			steeringOutput.DesiredSpeed = MovementSpeed::Regular;

			// Calculate the separation effects
			AddWallSeparationToSteering(steeringOutput, elapsedSec, 0.3f);
			AddAgentSeparationToSteering(steeringOutput, elapsedSec, 0.6f);

			break;
		}

		// Run the pure-wander counter, after which the agent should seek a random room
		m_RoomExploringCounter += elapsedSec;
		if (m_RoomExploringCounter >= m_CurrentRoomExploringTime)
		{
			PickNewRoomToSeek();
			m_RoomExploringCounter = 0.f;
			m_CurrentlySeekingNewRoom = true;
		}
		
		
		// Apply regular wander
		steeringOutput = m_pWanderBehavior->CalculateSteering(this, Vector2f(), elapsedSec);

		// Calculate the separation effects
		AddWallSeparationToSteering(steeringOutput, elapsedSec, 1.f);
		AddAgentSeparationToSteering(steeringOutput, elapsedSec, 0.8f);

		break;


	case Emotion::Fright:

		// Follow the flow-field to the exit, if the exit room is known
		if(m_Rooms[0].first)
			steeringOutput = m_pSeekBehavior->CalculateSteering(this, m_pExitFlowField->GetNextCellPosition(m_Position), elapsedSec);
		else
		{
			if (currentRoomChanged)
				PickNewRoomToSeek();
			steeringOutput = m_pSeekBehavior->CalculateSteering(this, m_pRoomsFlowFields[m_CurrentlySeekedRoomIdx]->GetNextCellPosition(m_Position), elapsedSec);
		}

		steeringOutput.DesiredSpeed = MovementSpeed::Sprint;

		// Calculate the separation effects
		AddWallSeparationToSteering(steeringOutput, elapsedSec, 0.3f);
		AddAgentSeparationToSteering(steeringOutput, elapsedSec, 0.4f);

		break;


	case Emotion::Panic:
		// Follow the flow-field to random rooms
		if (currentRoomChanged)
			PickNewRoomToSeek(false);
		steeringOutput = m_pSeekBehavior->CalculateSteering(this, m_pRoomsFlowFields[m_CurrentlySeekedRoomIdx]->GetNextCellPosition(m_Position), elapsedSec);
		randomFactorSteer = m_pWanderBehavior->CalculateSteering(this, Vector2f(), elapsedSec);
		if (m_pLastHazard != nullptr)
		{
			fleeFactorSteer = m_pFleeBehavior->CalculateSteering(this, m_pLastHazard->GetPosition(), elapsedSec);
			steeringOutput.MovementDirection = steeringOutput.MovementDirection * 0.2 + randomFactorSteer.MovementDirection * 0.5f + fleeFactorSteer.MovementDirection * 0.3f;
			steeringOutput.FacingDirection = steeringOutput.FacingDirection * 0.2 + randomFactorSteer.FacingDirection * 0.5f + fleeFactorSteer.FacingDirection * 0.3f;
		}
		else
		{
			steeringOutput.MovementDirection = steeringOutput.MovementDirection * 0.3 + randomFactorSteer.MovementDirection * 0.7f;
			steeringOutput.FacingDirection = steeringOutput.FacingDirection * 0.3 + randomFactorSteer.FacingDirection * 0.7f;
		}
		
		steeringOutput.MovementDirection.Normalize();
		steeringOutput.FacingDirection.Normalize();
		steeringOutput.DesiredSpeed = MovementSpeed::Sprint;

		// Calculate the separation effects
		AddWallSeparationToSteering(steeringOutput, elapsedSec, 1.f);
		AddAgentSeparationToSteering(steeringOutput, elapsedSec, 0.2f);

		break;


	default:
		break;
	}

	// Apply said steering
	m_MovementDirection = steeringOutput.MovementDirection;
	m_MovementDirection.Normalize();
	m_FacingDirection = steeringOutput.FacingDirection;
	m_FacingDirection.Normalize();

	m_PositionLastFrame = m_Position;
	switch (steeringOutput.DesiredSpeed)
	{
	case MovementSpeed::Slow:
		m_CurrentSpeed = m_SlowSpeed;
		break;
	case MovementSpeed::Regular:
		m_CurrentSpeed = m_RegularSpeed;
		break;
	case MovementSpeed::Sprint:
		m_CurrentSpeed = m_SprintSpeed;
		break;
	default:
		break;
	}
	m_Position += m_MovementDirection * m_CurrentSpeed * elapsedSec;


	// Run Inter-Agent Collison Handling
	for (int i = 0; i < m_pAgents->size(); i++)
	{
		if (m_pAgents->operator[](i) == this) continue;
		if (m_pAgents->operator[](i)->GetHasEscaped()) continue;

		if (m_pAgents->operator[](i)->GetPosition().Distance(m_Position) <= m_AgentDiameter)
		{
			auto bePushed = false;

			if (m_pAgents->operator[](i)->GetIsAuthority())
			{
				bePushed = true;
			}
			else
			{
				if (int(m_CurrentEmotion) != int(m_pAgents->operator[](i)->GetCurrentEmotion()))
				{
					if (int(m_CurrentEmotion) > int(m_pAgents->operator[](i)->GetCurrentEmotion()))
						bePushed = false;
					else
						bePushed = true;
				}
				else
				{
					if (m_CurrentEmotion == Emotion::Calm)
					{
						if (m_CurrentlySeekingNewRoom != m_pAgents->operator[](i)->GetCurrentlySeekingNewRoom())
							bePushed = !m_CurrentlySeekingNewRoom;
						else
							bePushed = m_EmotionalProfile.Extraversion < m_pAgents->operator[](i)->GetEmotionalProfile().Extraversion;
					}
					else bePushed = m_EmotionalProfile.Extraversion < m_pAgents->operator[](i)->GetEmotionalProfile().Extraversion;
				}
			}

			if (!bePushed)
			{
				auto pushForce = m_pAgents->operator[](i)->GetPosition() - m_Position;
				pushForce.Normalize();
				pushForce *= 5.f;
				m_pAgents->operator[](i)->SetPosition(m_pAgents->operator[](i)->GetPosition() + pushForce);
			}
			else
			{
				auto pushedForce = m_Position - m_pAgents->operator[](i)->GetPosition();
				pushedForce.Normalize();
				pushedForce *= 5.f;
				m_Position += pushedForce;
			}
		}
	}
}

void RegularAgent::ReactToHazardRange(Hazard* pHazard, float elapsedSec)
{
	if (m_HasEscaped) return;

	if (pHazard == nullptr) return;

	m_pLastHazard = pHazard;

	const auto distToHazard = pHazard->GetPosition().Distance(m_Position);
	auto distPerc = 1.f;
	if (distToHazard > 0.f) distPerc = distToHazard / (pHazard->GetEffectRange() / 2.f);
	const auto recievedFearAmount = (1.f - distPerc) * (1.f - m_EmotionalProfile.Neuroticism) * elapsedSec;
	const auto recievedPanicAmount = (1.f - distPerc) * m_EmotionalProfile.Neuroticism * elapsedSec;
	m_PercievedFright += recievedFearAmount * m_FrightSpreadRate * 5.f;
	if (m_CurrentEmotion != Emotion::Fright && m_PercievedFright >= 0.2f && m_PercievedPanic <= m_PercievedFright)
	{
		ResetAllBehaviors();
		m_CurrentEmotion = Emotion::Fright;
		PickNewRoomToSeek();
		m_EmotionalCooldownCounter = 0.f;
		m_OnEmotionalCooldown = true;
	}

	m_PercievedPanic += recievedPanicAmount * m_PanicSpreadRate * 5.f;
	if (m_CurrentEmotion != Emotion::Panic && m_PercievedPanic >= 0.2f && m_PercievedPanic >= m_PercievedFright)
	{
		ResetAllBehaviors();
		m_CurrentEmotion = Emotion::Panic;
		m_EmotionalCooldownCounter = 0.f;
		m_OnEmotionalCooldown = true;
	}
}

void RegularAgent::AddAgentSeparationToSteering(SteeringOutput& steeringOutput, float elapsedSec, float intensity)
{
	std::vector<Vector2f> agentFleeDir;
	std::vector<float> agentDist;
	float shortestDist = 10000000.f;
	for (int i = 0; i < m_pAgents->size(); i++)
	{
		if (m_pAgents->operator[](i) == this) continue;
		if (m_pAgents->operator[](i)->GetHasEscaped()) continue;

		const auto distToOtherAgent = m_pAgents->operator[](i)->GetPosition().Distance(m_Position);

		if (distToOtherAgent < shortestDist)
			shortestDist = distToOtherAgent;

		if (distToOtherAgent <= m_OtherAgentAwarenessRadius)
		{
			agentFleeDir.push_back(m_pFleeBehavior->CalculateSteering(this, m_pAgents->operator[](i)->GetPosition(), elapsedSec).MovementDirection);
			agentDist.push_back(distToOtherAgent);
		}
	}

	if (agentFleeDir.empty()) return;

	auto finalFleeDir = Vector2f(0.f, 0.f);
	auto combinedDist = 0.f;
	for (int i = 0; i < agentDist.size(); i++)
	{
		combinedDist += agentDist[i];
	}

	for (int i = 0; i < agentFleeDir.size(); i++) finalFleeDir += agentFleeDir[i] * (agentDist[i] / combinedDist);

	const auto steerPerc = (1.f - shortestDist / m_OtherAgentAwarenessRadius) * intensity;
	steeringOutput.MovementDirection = steeringOutput.MovementDirection * (1.f - steerPerc) + finalFleeDir * steerPerc;
	steeringOutput.MovementDirection.Normalize();
}

void RegularAgent::AddWallSeparationToSteering(SteeringOutput& steeringOutput, float elapsedSec, float intensity)
{
	std::vector<Vector2f> wallFleeDir;
	std::vector<float> wallDist;
	float shortestDist = 10000000.f;
	for (int i = 0; i < m_pWalls->size(); i++)
	{
		Vector2f pointOfContact;
		const auto distToWall = m_Position.Distance(m_pWalls->operator[](i)->GetBottomLeft(), m_pWalls->operator[](i)->GetSize(), pointOfContact);

		if (distToWall < shortestDist)
			shortestDist = distToWall;

		if (distToWall <= m_OtherAgentAwarenessRadius)
		{
			wallFleeDir.push_back(m_pFleeBehavior->CalculateSteering(this, pointOfContact, elapsedSec).MovementDirection);
			wallDist.push_back(distToWall);
		}
	}

	if (wallFleeDir.empty()) return;

	auto finalFleeDir = Vector2f(0.f, 0.f);
	auto combinedDist = 0.f;
	for (int i = 0; i < wallDist.size(); i++)
	{
		combinedDist += wallDist[i];
	}

	for (int i = 0; i < wallFleeDir.size(); i++) finalFleeDir += wallFleeDir[i] * (wallDist[i] / combinedDist);

	const auto steerPerc = (1.f - shortestDist / (m_OtherAgentAwarenessRadius)) * intensity;
	steeringOutput.MovementDirection = steeringOutput.MovementDirection * (1.f - steerPerc) + finalFleeDir * steerPerc;
	//steeringOutput.MovementDirection = finalFleeDir;
	steeringOutput.MovementDirection.Normalize();
}

void RegularAgent::GetEmotionallyInfected(float elapsedSec)
{
	if (m_OnEmotionalCooldown)
	{
		m_EmotionalCooldownCounter += elapsedSec;
		if (m_EmotionalCooldownCounter >= m_EmotionalCooldownTime)
		{
			m_EmotionalCooldownCounter = 0.f;
			m_OnEmotionalCooldown = false;
		}
	}
	// Slowly reduce percieved emotions according to neuroticism (a passive numbing effect)
	m_PercievedCalm -= (1.f - m_EmotionalProfile.Neuroticism) * elapsedSec * m_CalmAttenuationRate;
	if (m_PercievedCalm < 0.f) m_PercievedCalm = 0.f;
	m_PercievedFright -= m_EmotionalProfile.Neuroticism * elapsedSec * m_FrightAttenuationRate;
	if (m_PercievedFright < 0.f) m_PercievedFright = 0.f;
	m_PercievedPanic -= (1.f - m_EmotionalProfile.Neuroticism) * elapsedSec * m_PanicAttenuationRate;
	if (m_PercievedPanic < 0.f) m_PercievedPanic = 0.f;

	// Go over all agents and apply the influence from neaby ones
	for (int i = 0; i < m_pAgents->size(); i++)
	{
		// Early return if checking self
		auto* pCurAgent = m_pAgents->operator[](i);
		if (pCurAgent == this) continue;

		// Early return if the other agent's too distant
		const auto distBetweenAgents = pCurAgent->GetPosition().Distance(m_Position);
		if (distBetweenAgents > m_MaxEmotionalInfectionDist) continue;


		if (pCurAgent->GetIsAuthority() && pCurAgent->GetIsAlert())
		{
			// If the agent is a 3rd party agent and is alert
			// Increase Fright and decrease Panic according to distance and neuroticism
			// And give them temporary knowladge of the whole scene layout
			auto distPerc = 1.f;
			if (distBetweenAgents > 0.f) distPerc = distBetweenAgents / m_MaxEmotionalInfectionDist;
			const auto recievedEmotionAmountMax = (1.f - distPerc) * elapsedSec;
			const auto recievedEmotionAmount = recievedEmotionAmountMax * 0.5f + (recievedEmotionAmountMax * 0.5f) * (1.f - m_EmotionalProfile.Neuroticism);

			if (recievedEmotionAmount <= 0.f) continue;

			m_PercievedFright += recievedEmotionAmount * 1.5f;
			m_PercievedPanic -= recievedEmotionAmount * 1.5f;
			for (int roomIdx = 0; roomIdx < m_Rooms.size(); roomIdx++)
			{
				m_Rooms[roomIdx].first = true;
				m_Rooms[roomIdx].second = 0.f;
			}

		}
		else
		{
			// If it's a regular agent
			// Add to the appropriate percieved emotion value
			// According to the other agent's emotion, this agent's agreeableness, and the distance between both
			auto distPerc = 1.f;
			if (distBetweenAgents > 0.f) distPerc = distBetweenAgents / m_MaxEmotionalInfectionDist;
			const auto recievedEmotionAmount = (1.f - distPerc) * m_EmotionalProfile.Agreeableness * elapsedSec;

			if (recievedEmotionAmount <= 0.f) continue;

			switch (pCurAgent->GetCurrentEmotion())
			{
			case Emotion::Calm:
				m_PercievedCalm += recievedEmotionAmount * m_CalmSpreadRate;
				break;

			case Emotion::Fright:
				m_PercievedFright += recievedEmotionAmount * m_FrightSpreadRate * (1.f - m_EmotionalProfile.Neuroticism);
				break;

			case Emotion::Panic:
				m_PercievedPanic += recievedEmotionAmount * m_PanicSpreadRate * m_EmotionalProfile.Neuroticism;
				break;
			}

			// Proper algorithm - needs tweaking for my situation
			/*const auto emotionalExpIntensity = m_EmotionalProfile.Extraversion;
			auto recievedEmotionalEffect = (1.f - 1.f / (1.f + exp(pCurAgent->GetPosition().Distance(m_Position)))) *
				emotionalExpIntensity * pCurAgent->GetEmotionStrength() * GetEmotionStrength();*/
		}
	}


	if (!m_OnEmotionalCooldown)
	{
		// If any percieved emotion reaches the activation threshold, activate it
		if (m_CurrentEmotion != Emotion::Fright && m_PercievedFright >= 0.2f && m_PercievedPanic <= m_PercievedFright)
		{
			ResetAllBehaviors();
			m_CurrentEmotion = Emotion::Fright;
			PickNewRoomToSeek();
			m_EmotionalCooldownCounter = 0.f;
			m_OnEmotionalCooldown = true;
		}
		else if (m_CurrentEmotion != Emotion::Panic && m_PercievedPanic >= 0.2f && m_PercievedPanic >= m_PercievedFright)
		{
			ResetAllBehaviors();
			m_CurrentEmotion = Emotion::Panic;
			m_EmotionalCooldownCounter = 0.f;
			m_OnEmotionalCooldown = true;
		}
		else if (m_CurrentEmotion != Emotion::Calm && m_PercievedCalm >= 1.f && m_PercievedCalm > m_PercievedPanic && m_PercievedCalm > m_PercievedFright)
		{
			ResetAllBehaviors();
			m_CurrentEmotion = Emotion::Calm;
			m_EmotionalCooldownCounter = 0.f;
			m_OnEmotionalCooldown = true;
		}
	}
	
	// Make sure all emotions are capped between 0 and 1
	if (m_PercievedCalm > 1.f) m_PercievedCalm = 1.f;
	else if (m_PercievedCalm < 0.f) m_PercievedCalm = 0.f;
	if (m_PercievedFright > 1.f) m_PercievedFright = 1.f;
	else if (m_PercievedFright < 0.f) m_PercievedFright = 0.f;
	if (m_PercievedPanic > 1.f) m_PercievedPanic = 1.f;
	else if (m_PercievedPanic < 0.f) m_PercievedPanic = 0.f;


	// If the emotion that leads to the current behavior reaches 0 go back to calm
	if (m_CurrentEmotion != Emotion::Calm)
	{
		if (m_CurrentEmotion == Emotion::Fright && m_PercievedFright <= 0.0001f)
		{
			ResetAllBehaviors();
			m_CurrentEmotion = Emotion::Calm;
		}
		else if (m_CurrentEmotion == Emotion::Panic && m_PercievedPanic <= 0.0001f)
		{
			ResetAllBehaviors();
			m_CurrentEmotion = Emotion::Calm;
		}
	}
}

void RegularAgent::ResetAllBehaviors()
{
	//m_pWalkAwayCuriouslyBehavior->ResetBehavior();
	m_pWanderBehavior->ResetBehavior();
}

void RegularAgent::PickNewRoomToSeek(bool avoidKnownRooms)
{
	
	switch (m_CurrentRoomIdx)
	{
	case 0: // 1 4
		if(m_LastRoomBeforeCurrentIdx == 1) m_CurrentlySeekedRoomIdx = 4;
		else if(m_LastRoomBeforeCurrentIdx == 4) m_CurrentlySeekedRoomIdx = 1;
		else if (!avoidKnownRooms || (m_Rooms[1].first && m_Rooms[4].first) || (!m_Rooms[1].first && !m_Rooms[4].first))
		{
			m_CurrentlySeekedRoomIdx = rand() % 2;
			if (m_CurrentlySeekedRoomIdx == 0) m_CurrentlySeekedRoomIdx = 1;
			else  m_CurrentlySeekedRoomIdx = 4;
		}
		else if (m_Rooms[1].first) m_CurrentlySeekedRoomIdx = 4;
		else m_CurrentlySeekedRoomIdx = 1;
		break;

	case 1: // 0 2 3
		if (m_LastRoomBeforeCurrentIdx == 0)
		{
			if (m_Rooms[2].first) m_CurrentlySeekedRoomIdx = 3;
			else if (m_Rooms[3].first) m_CurrentlySeekedRoomIdx = 2;
			else m_CurrentlySeekedRoomIdx = rand() % 2 + 2;
		}
		else if (m_LastRoomBeforeCurrentIdx == 2)
		{
			if (m_Rooms[0].first) m_CurrentlySeekedRoomIdx = 3;
			else if (m_Rooms[3].first) m_CurrentlySeekedRoomIdx = 0;
			else 
			{
				m_CurrentlySeekedRoomIdx = rand() % 2;
				if(m_CurrentlySeekedRoomIdx == 1) m_CurrentlySeekedRoomIdx = 3;
			}
		}
		else if (m_LastRoomBeforeCurrentIdx == 3)
		{
			if (m_Rooms[0].first) m_CurrentlySeekedRoomIdx = 2;
			else if (m_Rooms[2].first) m_CurrentlySeekedRoomIdx = 0;
			else
			{
				m_CurrentlySeekedRoomIdx = rand() % 2;
				if (m_CurrentlySeekedRoomIdx == 1) m_CurrentlySeekedRoomIdx = 2;
			}
		}
		else if (!avoidKnownRooms || (m_Rooms[0].first && m_Rooms[2].first && m_Rooms[3].first) || (!m_Rooms[0].first && !m_Rooms[2].first && !m_Rooms[3].first))
		{
			m_CurrentlySeekedRoomIdx = rand() % 3;
			if (m_CurrentlySeekedRoomIdx == 1) m_CurrentlySeekedRoomIdx = 3;
		}
		else if (m_Rooms[0].first)
		{
			if (m_Rooms[2].first) m_CurrentlySeekedRoomIdx = 3;
			else if (m_Rooms[3].first) m_CurrentlySeekedRoomIdx = 2;
			else m_CurrentlySeekedRoomIdx = rand() % 2 + 2;
		}
		else if (m_Rooms[2].first)
		{
			if (m_Rooms[3].first) m_CurrentlySeekedRoomIdx = 0;
			else
			{
				m_CurrentlySeekedRoomIdx = rand() % 2;
				if (m_CurrentlySeekedRoomIdx == 1) m_CurrentlySeekedRoomIdx = 3;
			}
		}
		else
		{
			m_CurrentlySeekedRoomIdx = rand() % 2;
			if (m_CurrentlySeekedRoomIdx == 1) m_CurrentlySeekedRoomIdx = 2;
		}
		break;

	case 2: // 1 4
		if (m_LastRoomBeforeCurrentIdx == 1) m_CurrentlySeekedRoomIdx = 4;
		else if (m_LastRoomBeforeCurrentIdx == 4) m_CurrentlySeekedRoomIdx = 1;
		else if (!avoidKnownRooms || (m_Rooms[1].first && m_Rooms[4].first) || (!m_Rooms[1].first && !m_Rooms[4].first))
		{
			m_CurrentlySeekedRoomIdx = rand() % 2;
			if (m_CurrentlySeekedRoomIdx == 0) m_CurrentlySeekedRoomIdx = 4;
		}
		else if (m_Rooms[1].first) m_CurrentlySeekedRoomIdx = 4;
		else m_CurrentlySeekedRoomIdx = 1;
		break;

	case 3: // 1
		m_CurrentlySeekedRoomIdx = 1;
		break;
	case 4: // 0 2
		if (m_LastRoomBeforeCurrentIdx == 0) m_CurrentlySeekedRoomIdx = 2;
		else if (m_LastRoomBeforeCurrentIdx == 2) m_CurrentlySeekedRoomIdx = 0;
		else if (!avoidKnownRooms || (m_Rooms[0].first && m_Rooms[2].first) || (!m_Rooms[0].first && !m_Rooms[2].first))
		{
			m_CurrentlySeekedRoomIdx = rand() % 2;
			if (m_CurrentlySeekedRoomIdx == 1) m_CurrentlySeekedRoomIdx = 2;
		}
		else if (m_Rooms[0].first) m_CurrentlySeekedRoomIdx = 2;
		else m_CurrentlySeekedRoomIdx = 0;

		break;
	}
}

void RegularAgent::Escape()
{
	m_HasEscaped = true;
}