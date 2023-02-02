#pragma once
#include "CommonHeader.h"

class RegularAgent;
class Hazard;

class SteeringBehavior
{
public:
	SteeringBehavior() = default;
	virtual ~SteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) = 0;
	virtual SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) = 0;
	virtual SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) = 0;

	virtual void ResetBehavior() {}

	Vector3f GetColor() const { return m_Color; }

protected:
	Vector3f m_Color;
};




class Seek : public SteeringBehavior
{
public:
	Seek() { m_Color = Vector3f(37.f, 150.f, 118.f); }
	virtual ~Seek() = default;

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;
};


class Pursuit final : public Seek
{
public:
	Pursuit() { m_Color = Vector3f(234.f, 182.f, 118.f); }
	virtual ~Pursuit() = default;

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;
};


class Flee : public SteeringBehavior
{
public:
	Flee() { m_Color = Vector3f(50.f, 205.f, 50.f); }
	virtual ~Flee() = default;

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;

private:
	const float m_FleeRadius = 20.f;
};


class Evade final : public Flee
{
public:
	Evade() { m_Color = Vector3f(81.f, 189.f, 215.f); }
	~Evade() = default;

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;


private:
	const float m_EvadeRadius = 20.f;
};


class Arrive final : public SteeringBehavior
{
public:
	Arrive() { m_Color = Vector3f(239.f, 188.f, 109.f); }
	~Arrive() = default;

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;


private:
	const float m_TargetRadius = 4.f; // Radius in which agent recognizes having reached their destination
	const float m_SlowRadius = 10.f; // Radius in which agent starts slowing down
};


class Face final : public SteeringBehavior
{
public:
	Face() { m_Color = Vector3f(255.f, 153.f, 255.f); }
	~Face() = default;

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;
};


class Wander : public Seek
{
public:
	Wander() { m_Color = Vector3f(204.f, 204.f, 255.f); }
	~Wander() = default;

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;

	void ResetBehavior() override { m_WanderAngle = 0.f; }

protected:
	const float m_Offset = 20.f; // Offset (Agent direction)
	const float m_Radius = 2.f; // Wander Radius
	const float m_AngleChange = 20.f * (PI / 180.f); // Max WanderAngle change per frame
	float m_WanderAngle = 0.f; // Internal
};

class WalkAwayCuriously final : public Flee
{
public:
	WalkAwayCuriously() { m_Color = Vector3f(144.f, 238.f, 144.f); };
	~WalkAwayCuriously() = default;

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;

	void ResetBehavior() override
	{
		m_CurrentTurningSpeed = 0.f;
		m_CurrentCooldown = m_MinMaxCheckBehindCooldown.X + (m_MinMaxCheckBehindCooldown.Y - m_MinMaxCheckBehindCooldown.X) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
		m_Counter = 0.f;
		m_CheckingBack = true;
	}

private:
	const Vector2f m_MinMaxCheckBehindCooldown = Vector2f(0.5f, 0.7f);
	const Vector2f m_MinMaxWalkAwayCooldown = Vector2f(1.0f, 1.5f);
	const float m_MaxTurningSpeed = 1000.f;
	const float m_TurningAccelaration = 2000.f;
	float m_CurrentTurningSpeed = 0.f;
	float m_CurrentCooldown = m_MinMaxCheckBehindCooldown.X + (m_MinMaxCheckBehindCooldown.Y - m_MinMaxCheckBehindCooldown.X) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
	float m_Counter = 0.f;
	bool m_CheckingBack = true;
};

class Surround final : public Wander
{
public:
	Surround() { m_Color = Vector3f(152.f, 251.f, 152.f); };
	~Surround() = default;

	void ResetBehavior() override { m_WanderAngle = 0.f; }

	SteeringOutput CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec) override;
	SteeringOutput CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec) override;
};