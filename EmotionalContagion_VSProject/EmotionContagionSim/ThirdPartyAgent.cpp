#include "ThirdPartyAgent.h"
#include "GameEngine.h"
#include "Hazard.h"
#include "SteeringBehaviors.h"
#include "FlowField.h"

ThirdPartyAgent::ThirdPartyAgent(Vector2f initialPos, Vector2f initialDir, FlowField* pExitFlowField,
	std::vector<FlowField*> pRoomsFlowFields, std::vector<Quad*> pRoomsQuads, std::vector<Wall*>* pWalls, std::vector<RegularAgent*>* pAgents)
	: RegularAgent(initialPos, initialDir, OCEANProfile(1.f, 1.f, 1.f, 0.f, 0.f), pExitFlowField, pRoomsFlowFields, pRoomsQuads, pWalls, pAgents)
	, m_AlertState (AlertState::NotAlert)
{
	//for (int i = 0; i < m_Rooms.size(); i++)
	//	m_Rooms[i].first = true;
}

ThirdPartyAgent::~ThirdPartyAgent()
{
}

void ThirdPartyAgent::DrawForward(bool isPicked)
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
	switch (m_AlertState)
	{
	case AlertState::NotAlert:
		GAME_ENGINE->SetColor(RGB(0, 191, 255));
		break;
	case AlertState::SeekingAgentsInRoom:
		GAME_ENGINE->SetColor(RGB(148, 0, 211));
		break;
	case AlertState::ChangingRoom:
		GAME_ENGINE->SetColor(RGB(106, 90, 205));
		break;
	case AlertState::LeavingScene:
		GAME_ENGINE->SetColor(RGB(95, 158, 160));
		break;
	default:
		break;
	}
	GAME_ENGINE->FillOval(int(m_Position.X - m_AgentDiameter / 2.f), int(m_Position.Y - m_AgentDiameter / 2.f), int(m_AgentDiameter), int(m_AgentDiameter));

	// Draw the facing direction
	GAME_ENGINE->SetColor(RGB(0, 0, 0));
	GAME_ENGINE->DrawLine(int(m_Position.X), int(m_Position.Y),
		int(m_Position.X + m_FacingDirection.X * m_AgentDiameter / 2.f),
		int(m_Position.Y + m_FacingDirection.Y * m_AgentDiameter / 2.f));
}

void ThirdPartyAgent::DrawHUD()
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

		switch (m_AlertState)
		{
		case AlertState::NotAlert:
			GAME_ENGINE->SetColor(RGB(0, 191, 255));
			break;
		case AlertState::SeekingAgentsInRoom:
			GAME_ENGINE->SetColor(RGB(148, 0, 211));
			break;
		case AlertState::ChangingRoom:
			GAME_ENGINE->SetColor(RGB(106, 90, 205));
			break;
		case AlertState::LeavingScene:
			GAME_ENGINE->SetColor(RGB(95, 158, 160));
			break;
		default:
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
		GAME_ENGINE->DrawString(L"This agent does not have emotions", int(hudAgentCenter.X - 112.f), int(hudAgentCenter.Y + hudAgentDiameter + 15.f));
		switch (m_AlertState)
		{
		case AlertState::NotAlert:
			GAME_ENGINE->DrawString(L"Current Action: Patrol", int(hudAgentCenter.X - 63.f), int(hudAgentCenter.Y + hudAgentDiameter + 35.f));
			break;
		case AlertState::SeekingAgentsInRoom:
			GAME_ENGINE->DrawString(L"Current Action: Warn Others", int(hudAgentCenter.X - 88.f), int(hudAgentCenter.Y + hudAgentDiameter + 35.f));
			break;
		case AlertState::ChangingRoom:
			GAME_ENGINE->DrawString(L"Current Action: Change Room", int(hudAgentCenter.X - 94.f), int(hudAgentCenter.Y + hudAgentDiameter + 35.f));
			break;
		case AlertState::LeavingScene:
			GAME_ENGINE->DrawString(L"Current Action: Leave Scene", int(hudAgentCenter.X - 89.f), int(hudAgentCenter.Y + hudAgentDiameter + 35.f));
			break;
		default:
			break;
		}
	}

	// Draw Line 1
	const auto lineGap = 20;
	const auto line1PosY = int(hudAgentCenter.Y + hudAgentDiameter + 50.f) + 5 + lineGap;
	GAME_ENGINE->SetColor(RGB(220, 220, 220));
	GAME_ENGINE->DrawLine(GAME_ENGINE->GetWidth() - sideWindowWidth + 30 - 8, line1PosY, GAME_ENGINE->GetWidth() - 30 - 8, line1PosY);

	const auto oceanBarHeight = 15.f;
	const auto gapTitleToBar = 20;
	const auto gapToNextTitle = 4;
	const auto fullBarGap = gapTitleToBar + oceanBarHeight + gapToNextTitle;
	const auto line2PosY = line1PosY + lineGap + fullBarGap * 4 + gapTitleToBar + oceanBarHeight + lineGap;

	GAME_ENGINE->DrawString(L"This agent is an authority figure", int(hudAgentCenter.X - 110.f), int(line1PosY + (line2PosY - line1PosY) / 2.f - 20));
	GAME_ENGINE->DrawString(L"They only behave professionally", int(hudAgentCenter.X - 113.f), int(line1PosY + (line2PosY - line1PosY) / 2.f));
	GAME_ENGINE->DrawString(L"They do not have an emotional profile", int(hudAgentCenter.X - 125.f), int(line1PosY + (line2PosY - line1PosY) / 2.f + 20));

	// Draw Line 2
	GAME_ENGINE->SetColor(RGB(220, 220, 220));
	GAME_ENGINE->DrawLine(GAME_ENGINE->GetWidth() - sideWindowWidth + 30 - 7, line2PosY, GAME_ENGINE->GetWidth() - 30 - 7, line2PosY);


	// Write emotions accumulation
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(L"Accumulated Emotions", int(hudAgentCenter.X - 76.f), line2PosY + lineGap);
	GAME_ENGINE->SetColor(RGB(180, 180, 180));
	GAME_ENGINE->DrawString(L"This agent is not influenced by others", int(hudAgentCenter.X - 125.f), line2PosY + lineGap + 50);


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
	
	GAME_ENGINE->SetColor(RGB(30, 150, 30));
	GAME_ENGINE->FillRect(mapLeft, mapTop + mapSizeY / 5 * 4, mapSizeX, mapSizeY / 5);
	GAME_ENGINE->FillRect(mapLeft, mapTop, mapSizeX / 4, mapSizeY / 5 * 4);
	GAME_ENGINE->FillRect(mapLeft + mapSizeX / 4, mapTop, mapSizeX / 2, mapSizeY / 5 * 2);
	GAME_ENGINE->FillRect(mapLeft + mapSizeX / 4, mapTop + mapSizeY / 5 * 2, mapSizeX / 2, mapSizeY / 5 * 2);
	GAME_ENGINE->FillRect(mapLeft + mapSizeX - mapSizeX / 4 - 1, mapTop, mapSizeX / 4, mapSizeY / 5 * 4);
	GAME_ENGINE->SetColor(RGB(0, 0, 0));
	
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
	GAME_ENGINE->DrawString(L"Max Memory Time: Infinite", GAME_ENGINE->GetWidth() - sideWindowWidth / 2 - 95, mapTop + mapSizeY + 10);
}

void ThirdPartyAgent::Update(float elapsedSec)
{
	if (m_HasEscaped) return;

	// Check if there's any worried agents nearby
	GetEmotionallyInfected(elapsedSec);

	// Update current room
	auto currentRoomChanged = false;
	if (!m_pRoomsQuads[m_CurrentRoomIdx]->IsOverlappingCircle(m_Position, m_AgentDiameter / 2.f))
	{
		for (int i = 0; i < m_pRoomsQuads.size(); i++)
		{
			if (m_pRoomsQuads[i]->IsOverlappingCircle(m_Position, m_AgentDiameter / 2.f))
			{
				m_Rooms[i].first = true;
				m_CurrentRoomIdx = i;
				currentRoomChanged = true;
				break;
			}
		}
	}

	// Calculate the desired steering, according to if alert
	auto steeringOutput = SteeringOutput();

	std::vector<RegularAgent*> pAgentsToHelp;
	const auto curRoomQuad = m_pRoomsQuads[m_CurrentRoomIdx];
	RegularAgent* pAgentLeftInScene = nullptr;

	switch (m_AlertState)
	{
	case AlertState::NotAlert:
		// Make sure the seeked room idx isn't invalid
		if (m_CurrentlySeekedRoomIdx < 0)
		{
			for (int i = 0; i < m_pRoomsQuads.size(); i++)
			{
				if (m_pRoomsQuads[i]->IsOverlappingCircle(m_Position, m_AgentDiameter / 2.f))
				{
					m_Rooms[i].first = true;
					m_CurrentRoomIdx = i;
					break;
				}
			}
			PickNewRoomToSeek(false);
		}

		// Check if the agent has reached its center
		if (m_Position.IsWithinCircleArea(m_pRoomsQuads[m_CurrentlySeekedRoomIdx]->BottomLeft + m_pRoomsQuads[m_CurrentlySeekedRoomIdx]->Size / 2.f, 30.f))
		{
			// If so, pick a new room to seek
			PickNewRoomToSeek(false);
		}

		// Follow the flow-field to the target room
		steeringOutput = m_pSeekBehavior->CalculateSteering(this, m_pRoomsFlowFields[m_CurrentlySeekedRoomIdx]->GetNextCellPosition(m_Position), elapsedSec);
		steeringOutput.DesiredSpeed = MovementSpeed::Regular;

		// Calculate the separation effects
		AddWallSeparationToSteering(steeringOutput, elapsedSec, 0.3f);
		AddAgentSeparationToSteering(steeringOutput, elapsedSec, 0.5f);
		break;


	case AlertState::SeekingAgentsInRoom:
		// Go over all agents and save the ones that are non-evacuating and in the current room
		for (int i = 0; i < m_pAgents->size(); i++)
		{
			auto* pAgent = m_pAgents->operator[](i);

			if (pAgent == this) continue;
			if (pAgent->GetHasEscaped()) continue;
			if (pAgent->GetIsAuthority()) continue;
			if (pAgent->GetCurrentEmotion() == Emotion::Fright) continue;
			if (!pAgent->GetPosition().IsWithinRectArea(curRoomQuad->BottomLeft, curRoomQuad->Size)) continue;

			pAgentsToHelp.push_back(pAgent);
		}

		// If there are no more non-evacuating agents left in the room
		if (pAgentsToHelp.empty())
		{
			// Save room as checked - the float of the rooms pair is used as a "checked" bool (anything above or equal to 1.f is true)
			m_Rooms[m_CurrentRoomIdx].second = 1.1f;

			// Look for more agents aside from the authority in the scene
			pAgentLeftInScene = nullptr;
			for (int i = 0; i < m_pAgents->size(); i++)
			{
				auto* pAgent = m_pAgents->operator[](i);

				if (pAgent == this) continue;
				if (pAgent->GetHasEscaped()) continue;
				if (pAgent->GetIsAuthority()) continue;
				if (pAgent->GetCurrentEmotion() == Emotion::Fright) continue;

				pAgentLeftInScene = pAgent;
				break;
			}

			// If none are found, leave the scene
			if (pAgentLeftInScene == nullptr)
			{
				m_AlertState = AlertState::LeavingScene;
				break;
			}

			// If there are agents left
			// Change room
			m_TargetRoomToCheck = GetClosestUncheckedRoomIdx();
			m_AlertState = AlertState::ChangingRoom;
			break;
		}
			
		// If there still are non-evacuating agents left in the room
		{
			// Pick the closest and seek them out (to influence them into evacuating)
			RegularAgent* closestAgent = nullptr;
			float closestAgentDist = FLT_MAX;
			for (auto* pAgent : pAgentsToHelp)
			{
				const float agentDist = m_Position.Distance(pAgent->GetPosition());
				if (agentDist < closestAgentDist)
				{
					closestAgent = pAgent;
					closestAgentDist = agentDist;
				}
			}

			if (closestAgent != nullptr)
			{
				steeringOutput = m_pSeekBehavior->CalculateSteering(this, closestAgent->GetPosition(), elapsedSec);
				steeringOutput.DesiredSpeed = MovementSpeed::Sprint;
			}
		}
		break;


	case AlertState::ChangingRoom:

		// Look for more agents aside from the authority in the scene
		pAgentLeftInScene = nullptr;
		for (int i = 0; i < m_pAgents->size(); i++)
		{
			auto* pAgent = m_pAgents->operator[](i);

			if (pAgent == this) continue;
			if (pAgent->GetHasEscaped()) continue;
			if (pAgent->GetIsAuthority()) continue;
			if (pAgent->GetCurrentEmotion() == Emotion::Fright) continue;

			pAgentLeftInScene = pAgent;
			break;
		}
		// If none are found, leave the scene
		if (pAgentLeftInScene == nullptr)
		{
			m_AlertState = AlertState::LeavingScene;
			break;
		}

		// Check if the room to check is invalid
		if (m_TargetRoomToCheck < 0)
		{
			// If so try to get an unchecked room again
			m_TargetRoomToCheck = GetClosestUncheckedRoomIdx();
			
			// And if it still comes off as invalid
			if (m_TargetRoomToCheck < 0)
			{
				// Look for more agents aside from the authority in the scene
				pAgentLeftInScene = nullptr;
				for (int i = 0; i < m_pAgents->size(); i++)
				{
					auto* pAgent = m_pAgents->operator[](i);

					if (pAgent == this) continue;
					if (pAgent->GetHasEscaped()) continue;
					if (pAgent->GetIsAuthority()) continue;
					if (pAgent->GetCurrentEmotion() == Emotion::Fright) continue;

					pAgentLeftInScene = pAgent;
					break;
				}

				// If none are found
				if (pAgentLeftInScene == nullptr)
				{
					m_AlertState = AlertState::LeavingScene;
					break;
				}
					
				// If there is at least one more
				// Save the room they're on as the target and unchecked
				auto agentInRoom = false;
				for (int i = 0; i < m_pRoomsQuads.size(); i++)
				{
					if (pAgentLeftInScene->GetPosition().IsWithinRectArea(m_pRoomsQuads[i]->BottomLeft, m_pRoomsQuads[i]->Size))
					{
						agentInRoom = true;
						m_TargetRoomToCheck = i;
						m_Rooms[m_TargetRoomToCheck].second = 1.1f;
						break;
					}
				}

				if (agentInRoom) break;
				
				// If they're in-between rooms, get the closest room instead
				int closestRoomIdx = -1;
				float closestRoomDist = FLT_MAX;
				for (int i = 0; i < m_pRoomsQuads.size(); i++)
				{
					auto roomDist = pAgentLeftInScene->GetPosition().Distance(m_pRoomsQuads[i]->BottomLeft + m_pRoomsQuads[i]->Size / 2.f);
					if (roomDist < closestRoomDist)
					{
						closestRoomIdx = i;
						closestRoomDist = roomDist;
					}
				}

				if (closestRoomIdx < 0) break;

				// And save said room as the target and unchecked
				m_TargetRoomToCheck = closestRoomIdx;
				m_Rooms[m_TargetRoomToCheck].second = 1.1f;
			}
		}

		// If there's still a valid unchecked room
			
		// Check if the agent has reached its center
		if (m_Position.IsWithinRectArea(m_pRoomsQuads[m_TargetRoomToCheck]->BottomLeft, m_pRoomsQuads[m_TargetRoomToCheck]->Size))
		{
			// Make them change to agent seeking state
			m_AlertState = AlertState::SeekingAgentsInRoom;
			break;
		}

		// If they're still not there yet,
		// Just sprint towards the room
		steeringOutput = m_pSeekBehavior->CalculateSteering(this, m_pRoomsFlowFields[m_TargetRoomToCheck]->GetNextCellPosition(m_Position), elapsedSec);
		steeringOutput.DesiredSpeed = MovementSpeed::Sprint;			
		break;


	case AlertState::LeavingScene:
		// Look for more agents aside from the authority in the scene
		pAgentLeftInScene = nullptr;
		for (int i = 0; i < m_pAgents->size(); i++)
		{
			auto* pAgent = m_pAgents->operator[](i);

			if (pAgent == this) continue;
			if (pAgent->GetHasEscaped()) continue;
			if (pAgent->GetIsAuthority()) continue;
			if (pAgent->GetCurrentEmotion() == Emotion::Fright) continue;

			pAgentLeftInScene = pAgent;
			break;
		}
		// If any are found, go back to seeking agents
		if (pAgentLeftInScene != nullptr)
		{
			m_AlertState = AlertState::SeekingAgentsInRoom;
			break;
		}

		// Follow the flow-field to the target room
		steeringOutput = m_pSeekBehavior->CalculateSteering(this, m_pExitFlowField->GetNextCellPosition(m_Position), elapsedSec);
		steeringOutput.DesiredSpeed = MovementSpeed::Sprint;
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
			auto pushForce = m_pAgents->operator[](i)->GetPosition() - m_Position;
			pushForce.Normalize();
			pushForce *= 5.f;
			m_pAgents->operator[](i)->SetPosition(m_pAgents->operator[](i)->GetPosition() + pushForce);
		}
	}
}













void ThirdPartyAgent::ReactToHazardRange(Hazard* pHazard, float elapsedSec)
{
	if (m_HasEscaped) return;

	if (pHazard == nullptr) return;

	if (m_AlertState != AlertState::NotAlert) return;
	
	BecomeAlert();
}

void ThirdPartyAgent::GetEmotionallyInfected(float elapsedSec)
{
	if (m_HasEscaped) return;

	if (m_AlertState != AlertState::NotAlert) return;
	
	// Go over all agents
	for (int i = 0; i < m_pAgents->size(); i++)
	{
		// Continue if checking self
		auto* pCurAgent = m_pAgents->operator[](i);
		if (pCurAgent == this) continue;

		// Continue if the agent is a 3rd party
		if (pCurAgent->GetIsAuthority()) continue;

		// Continue if the other agent's too distant
		const auto distBetweenAgents = pCurAgent->GetPosition().Distance(m_Position);
		if (distBetweenAgents > m_MaxEmotionalInfectionDist) continue;

		// If the agent is panicked or frightened, become alert
		if (pCurAgent->GetCurrentEmotion() != Emotion::Calm)
		{
			BecomeAlert();
			break;
		}
	}
}

void ThirdPartyAgent::BecomeAlert()
{
	ResetAllBehaviors();
	m_AlertState = AlertState::SeekingAgentsInRoom;
}

int ThirdPartyAgent::GetClosestUncheckedRoomIdx()
{
	int randomInt;
	switch (m_CurrentRoomIdx)
	{
	case 0: // 1 4
		if (m_LastRoomBeforeCurrentIdx == 1) return 4;
		if (m_LastRoomBeforeCurrentIdx == 4) return 1;
		if ((m_Rooms[1].second >= 1.f && m_Rooms[4].second >= 1.f) || (m_Rooms[1].second < 1.f && m_Rooms[4].second < 1.f))
		{
			randomInt = rand() % 2;
			if (randomInt == 0) return 1;
			return 4;
		}
		if (m_Rooms[1].second >= 1.f) return 4;
		return 1;

	case 1: // 0 2 3
		if (m_LastRoomBeforeCurrentIdx == 0)
		{
			if (m_Rooms[2].second >= 1.f) return 3;
			if (m_Rooms[3].second >= 1.f) return 2;
			return rand() % 2 + 2;
		}
		if (m_LastRoomBeforeCurrentIdx == 2)
		{
			if (m_Rooms[0].second >= 1.f) return 3;
			if (m_Rooms[3].second >= 1.f) return 0;
			randomInt = rand() % 2;
			if (randomInt == 1) return 3;
			return randomInt;
		}
		if (m_LastRoomBeforeCurrentIdx == 3)
		{
			if (m_Rooms[0].second >= 1.f) return 2;
			if (m_Rooms[2].second >= 1.f) return 0;
			randomInt = rand() % 2;
			if (randomInt == 1) return 2;
			return randomInt;
		}
		if ((m_Rooms[0].second >= 1.f && m_Rooms[2].second >= 1.f && m_Rooms[3].second >= 1.f) ||
			(m_Rooms[0].second < 1.f && m_Rooms[2].second < 1.f && m_Rooms[3].second < 1.f))
		{
			randomInt = rand() % 3;
			if (randomInt == 1) return 3;
			return randomInt;
		}
		if (m_Rooms[0].second >= 1.f)
		{
			if (m_Rooms[2].second >= 1.f) return 3;
			if (m_Rooms[3].second >= 1.f) return 2;
			return rand() % 2 + 2;
		}
		if (m_Rooms[2].second >= 1.f)
		{
			if (m_Rooms[3].second >= 1.f) return 0;
			else
			{
				randomInt = rand() % 2;
				if (randomInt == 1) return 3;
				return randomInt;
			}
		}
		randomInt = rand() % 2;
		if (randomInt == 1) return 2;
		return randomInt;

	case 2: // 1 4
		if (m_LastRoomBeforeCurrentIdx == 1) return 4;
		if (m_LastRoomBeforeCurrentIdx == 4) return 1;
		if ((m_Rooms[1].second >= 1.f && m_Rooms[4].second >= 1.f) || (m_Rooms[1].second < 1.f && m_Rooms[4].second < 1.f))
		{
			randomInt = rand() % 2;
			if (randomInt == 0) return 4;
			return randomInt;
		}
		if (m_Rooms[1].second >= 1.f) return 4;
		return 1;

	case 3: // 1
		return 1;

	case 4: // 0 2
		if (m_LastRoomBeforeCurrentIdx == 0) return 2;
		if (m_LastRoomBeforeCurrentIdx == 2) return 0;
		if ((m_Rooms[0].second >= 1.f && m_Rooms[2].second >= 1.f) || (m_Rooms[0].second < 1.f && m_Rooms[2].second < 1.f))
		{
			randomInt = rand() % 2;
			if (randomInt == 1) return 2;
			return randomInt;
		}
		if (m_Rooms[0].second >= 1.f) return 2;
		return 0;
	}
}

