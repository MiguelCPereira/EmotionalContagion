#pragma once
#include "CommonHeader.h"
#include <vector>

class RegularAgent;

class Hazard
{
public:
	Hazard(Vector2f initialPos, std::vector<RegularAgent*> pAgents, Vector2f size = Vector2f(20.f, 20.f), float effectRange = 80.f, float dangerLevel = 5.0f);
	virtual ~Hazard();

	Hazard(const Hazard&) = delete;
	Hazard(Hazard&&) = delete;
	Hazard& operator=(const Hazard&) = delete;
	Hazard& operator=(const Hazard&&) = delete;

	virtual void Draw();
	virtual void Update(float elapsedSec);

	void ResetAgents(std::vector<RegularAgent*> pAgents);

	Vector2f GetPosition() const { return m_Position; }
	float GetEffectRange() const { return m_EffectRange; }

protected:
	Vector2f m_Position;
	Vector2f m_Size;
	float m_EffectRange;
	const float m_DangerLevel; // Used for contagion calculation
	std::vector<RegularAgent*> m_pAgents;
};

class MovingHazard : public Hazard
{
public:
	MovingHazard(const std::vector<Vector2f>& targetPositions, std::vector<RegularAgent*> pAgents, Vector2f size = Vector2f(20.f, 20.f),
		float speed = 20.f, bool loop = true, float pointIconSize = 5.f, float effectRange = 80.f, float dangerLevel = 5.0f);
	~MovingHazard();

	void Draw() override;
	void Update(float elapsedSec) override;

	Vector2f GetDirection() const { return m_CurrentDirection; }
	float GetSpeed() const { return m_Speed; }

private:
	const std::vector<Vector2f> m_TargetPositions;
	float m_Speed;
	bool m_LoopBetweenPoints;
	float m_PointIconSize;

	int m_CurrentPoint;
	Vector2f m_CurrentDirection;
	float m_TraveledDistance;
	float m_TotalDistBetweenPoints;
	bool m_MovingForward;
};

