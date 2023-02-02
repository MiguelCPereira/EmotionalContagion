#include "Wall.h"
#include "GameEngine.h"
#include "RegularAgent.h"

Wall::Wall(Vector2f initialPos, std::vector<RegularAgent*> pAgents, Vector2f size, bool visible)
	: m_Position(initialPos)
	, m_Size(size)
	, m_pAgents(pAgents)
	, m_Visible(visible)
	, m_BounceBackAmount(1.5f)
{
}

Wall::~Wall()
{
}

void Wall::Draw()
{
	if (!m_Visible) return;

	GAME_ENGINE->SetColor(RGB(200, 200, 200));
	GAME_ENGINE->FillRect(int(m_Position.X - m_Size.X / 2.f), int(m_Position.Y - m_Size.Y / 2.f), int(m_Size.X), int(m_Size.Y));
}

void Wall::Update()
{
	for (auto* pAgent : m_pAgents)
	{
		if ((pAgent->GetPosition() + Vector2f(0.f, pAgent->GetAgentDiameter() / 2.f)).IsWithinRectArea(Vector2f(m_Position.X - m_Size.X / 2.f, m_Position.Y - m_Size.Y / 2.f), m_Size) ||
			(pAgent->GetPosition() - Vector2f(0.f, pAgent->GetAgentDiameter() / 2.f)).IsWithinRectArea(Vector2f(m_Position.X - m_Size.X / 2.f, m_Position.Y - m_Size.Y / 2.f), m_Size) ||
			(pAgent->GetPosition() + Vector2f(pAgent->GetAgentDiameter() / 2.f, 0.f)).IsWithinRectArea(Vector2f(m_Position.X - m_Size.X / 2.f, m_Position.Y - m_Size.Y / 2.f), m_Size) ||
			(pAgent->GetPosition() - Vector2f(pAgent->GetAgentDiameter() / 2.f, 0.f)).IsWithinRectArea(Vector2f(m_Position.X - m_Size.X / 2.f, m_Position.Y - m_Size.Y / 2.f), m_Size))
		{
			auto finalPosition = pAgent->GetPosition();

			const auto distanceToTopBound = m_Position.Y + m_Size.Y / 2.f - (pAgent->GetPositionLastFrame().Y + pAgent->GetAgentDiameter() / 2.f);
			const auto distanceToBotBound = (pAgent->GetPositionLastFrame().Y - pAgent->GetAgentDiameter() / 2.f) - m_Position.Y + m_Size.Y / 2.f;
			const auto distanceToRightBound = m_Position.X + m_Size.X / 2.f - (pAgent->GetPositionLastFrame().X + pAgent->GetAgentDiameter() / 2.f);
			const auto distanceToLeftBound = (pAgent->GetPositionLastFrame().X - pAgent->GetAgentDiameter() / 2.f) - m_Position.X + m_Size.X / 2.f;

			if (distanceToTopBound < distanceToBotBound && distanceToTopBound < distanceToRightBound && distanceToTopBound < distanceToLeftBound)
				finalPosition.Y = m_Position.Y + m_Size.Y / 2.f + pAgent->GetAgentDiameter() / 2.f + m_BounceBackAmount;
			else if(distanceToBotBound < distanceToTopBound && distanceToBotBound < distanceToRightBound && distanceToBotBound < distanceToLeftBound)
				finalPosition.Y = m_Position.Y - m_Size.Y / 2.f - pAgent->GetAgentDiameter() / 2.f - m_BounceBackAmount;
			else if (distanceToRightBound < distanceToLeftBound && distanceToRightBound < distanceToTopBound && distanceToRightBound < distanceToBotBound)
				finalPosition.X = m_Position.X + m_Size.X / 2.f + pAgent->GetAgentDiameter() / 2.f + m_BounceBackAmount;
			else finalPosition.X = m_Position.X - m_Size.X / 2.f - pAgent->GetAgentDiameter() / 2.f - m_BounceBackAmount;

			pAgent->SetPosition(finalPosition);
		}
	}
}

void Wall::ResetAgents(std::vector<RegularAgent*> pAgents)
{
	m_pAgents = pAgents;
}