#include "Hazard.h"
#include "GameEngine.h"
#include "RegularAgent.h"

Hazard::Hazard(Vector2f initialPos, std::vector<RegularAgent*> pAgents, Vector2f size, float effectRange, float dangerLevel)
	: m_Position(initialPos)
	, m_Size(size)
	, m_EffectRange(effectRange)
	, m_DangerLevel(dangerLevel)
	, m_pAgents(pAgents)
{
}

Hazard::~Hazard()
{
}

void Hazard::Draw()
{
	// Draw the effect area
	GAME_ENGINE->SetColor(RGB(255 / 4, 0, 0));
	GAME_ENGINE->FillOval(int(m_Position.X - m_EffectRange / 2.f), int(m_Position.Y - m_EffectRange / 2.f), int(m_EffectRange), int(m_EffectRange));

	// Draw the hazard
	GAME_ENGINE->SetColor(RGB(255, 0, 0));
	GAME_ENGINE->FillRoundRect(int(m_Position.X - m_Size.X / 2.f), int(m_Position.Y - m_Size.Y / 2.f), int(m_Size.X), int(m_Size.Y), 5);
}

void Hazard::Update(float elapsedSec)
{
	for (auto* pAgent : m_pAgents)
	{
		if (pAgent->GetPosition().Distance(m_Position) <= m_EffectRange / 2.f + pAgent->GetAgentDiameter() / 2.f)
		{
			pAgent->ReactToHazardRange(this, elapsedSec);
		}
	}
}

void Hazard::ResetAgents(std::vector<RegularAgent*> pAgents)
{
	m_pAgents = pAgents;
}

MovingHazard::MovingHazard(const std::vector<Vector2f>& targetPositions, std::vector<RegularAgent*> pAgents, Vector2f size,
	float speed, bool loop, float pointIconSize, float effectRange, float dangerLevel)
	: Hazard(targetPositions[0], pAgents, size, effectRange, dangerLevel)
	, m_TargetPositions(targetPositions)
	, m_Speed(speed)
	, m_LoopBetweenPoints(loop)
	, m_PointIconSize(pointIconSize)
	, m_CurrentPoint(0)
	, m_TraveledDistance()
	, m_TotalDistBetweenPoints()
	, m_MovingForward(true)
{
	if (targetPositions.empty()) return;
	m_Position = targetPositions[0];

	if (targetPositions.size() <= 1) return;
	m_TotalDistBetweenPoints = targetPositions[0].Distance(targetPositions[1]);
	m_CurrentDirection = targetPositions[1] - targetPositions[0];
}

MovingHazard::~MovingHazard()
{
}

void MovingHazard::Draw()
{
	Hazard::Draw();

	// Draw the path points
	GAME_ENGINE->SetColor(RGB(255, 0, 0));
	for (const auto& targetPos : m_TargetPositions)
	{
		GAME_ENGINE->FillOval(int(m_Position.X - m_PointIconSize / 2.f), int(m_Position.Y - m_PointIconSize / 2.f), int(m_PointIconSize), int(m_PointIconSize));
	}
}

void MovingHazard::Update(float elapsedSec)
{
	if (m_TargetPositions.size() <= 1) return;

	// Move the hazard towards the current target
	m_Position += m_CurrentDirection * m_Speed * elapsedSec;
	m_TraveledDistance += m_Speed * elapsedSec;

	// Switch target if the current target has been reached
	if (m_TotalDistBetweenPoints >= m_TraveledDistance)
	{
		m_TraveledDistance = 0.f;

		auto oldPoint = m_CurrentPoint;
		if (m_MovingForward)
		{
			m_CurrentPoint++;
			if (m_CurrentPoint >= m_TargetPositions.size())
			{
				m_MovingForward = false;
				m_CurrentPoint = int(m_TargetPositions.size()) - 2;
			}
		}
		else
		{
			m_CurrentPoint--;
			if (m_CurrentPoint < 0)
			{
				m_MovingForward = true;
				m_CurrentPoint = 1;
			}
		}

		m_CurrentDirection = m_TargetPositions[m_CurrentPoint] - m_TargetPositions[oldPoint];
		m_TotalDistBetweenPoints = m_TargetPositions[m_CurrentPoint].Distance(m_TargetPositions[oldPoint]);
	}
}
