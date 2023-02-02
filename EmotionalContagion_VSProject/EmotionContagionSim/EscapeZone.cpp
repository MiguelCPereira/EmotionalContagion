#include "EscapeZone.h"
#include "GameEngine.h"
#include "RegularAgent.h"

EscapeZone::EscapeZone(Vector2f initialPos, std::vector<RegularAgent*> pAgents, Vector2f size)
	: m_Position(initialPos)
	, m_Size(size)
	, m_pAgents(pAgents)
{
}

EscapeZone::~EscapeZone()
{
}

void EscapeZone::Draw(bool exitSelected)
{
	// Draw the area
	if(exitSelected) GAME_ENGINE->SetColor(RGB(34 * 1.5, 135 * 1.5, 101 * 1.5));
	else GAME_ENGINE->SetColor(RGB(34, 135, 101));
	GAME_ENGINE->FillRoundRect(int(m_Position.X - m_Size.X / 2.f), int(m_Position.Y - m_Size.Y / 2.f), int(m_Size.X), int(m_Size.Y), 5);
}

void EscapeZone::Update()
{
	for (auto* pAgent : m_pAgents)
	{
		if (pAgent->GetPosition().IsWithinRectArea(Vector2f(m_Position.X - m_Size.X / 2.f, m_Position.Y - m_Size.Y / 2.f), m_Size))
			pAgent->Escape();
	}
}

void EscapeZone::ResetAgents(std::vector<RegularAgent*> pAgents)
{
	m_pAgents = pAgents;
}
