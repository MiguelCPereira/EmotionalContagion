#include "SteeringBehaviors.h"
#include "RegularAgent.h"
#include "Hazard.h"
#include <iostream>

SteeringOutput Seek::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};
	steering.MovementDirection = staticTarget - pAgent->GetPosition();
	steering.MovementDirection.Normalize();
	steering.FacingDirection = steering.MovementDirection;
	steering.DesiredSpeed = MovementSpeed::Regular;
	steering.DesiredSpeedAmount = 1.f;
	return steering;
}

SteeringOutput Seek::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pMovingTarget->GetPosition(), elapsedSec);
}

SteeringOutput Seek::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pHazardTarget->GetPosition(), elapsedSec);
}

SteeringOutput Pursuit::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	std::cout << "A static target can't be pursuited\n";
	auto steering = SteeringOutput{};
	steering.IsValid = false;
	return steering;
}

SteeringOutput Pursuit::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	if (pMovingTarget == nullptr)
	{
		steering.IsValid = false;
		return steering;
	}

	const float targetDistance = pAgent->GetPosition().Distance(pMovingTarget->GetPosition()); // Calculate the distance between the agent and the target
	const float predictionTime = targetDistance / pAgent->GetCurrentSpeed(); // Calculate the prediction time
	const Vector2f predictedTarget = pMovingTarget->GetPosition() + pMovingTarget->GetMovementDirection() * pMovingTarget->GetCurrentSpeed() * predictionTime;
	
	return Seek::CalculateSteering(pAgent, predictedTarget, elapsedSec);
}

SteeringOutput Pursuit::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	if (pHazardTarget == nullptr)
	{
		steering.IsValid = false;
		return steering;
	}

	// If its a regular hazard, use the fixed target function
	auto* pMovingHazard = dynamic_cast<MovingHazard*>(pHazardTarget);
	if (pMovingHazard == nullptr)
		return CalculateSteering(pAgent, pHazardTarget->GetPosition(), elapsedSec);


	const float targetDistance = pAgent->GetPosition().Distance(pMovingHazard->GetPosition()); // Calculate the distance between the agent and the target
	const float predictionTime = targetDistance / pAgent->GetCurrentSpeed(); // Calculate the prediction time
	const Vector2f predictedTarget = pMovingHazard->GetPosition() + pMovingHazard->GetDirection() * pMovingHazard->GetSpeed() * predictionTime;
	return Seek::CalculateSteering(pAgent, predictedTarget, elapsedSec);
}

SteeringOutput Flee::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	float targetDistance = pAgent->GetPosition().Distance(staticTarget); // Calculate the distance between the agent and the target

	auto steering = SteeringOutput{};

	//if (targetDistance > m_FleeRadius) // If the distance's bigger the the FleeRadius, stop moving
	//{
	//	steering.IsValid = false;
	//	return steering;
	//}

	steering.MovementDirection = pAgent->GetPosition() - staticTarget;
	steering.MovementDirection.Normalize();
	steering.FacingDirection = steering.MovementDirection;
	steering.DesiredSpeed = MovementSpeed::Sprint;
	steering.DesiredSpeedAmount = 1.f;
	return steering;
}

SteeringOutput Flee::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pMovingTarget->GetPosition(), elapsedSec);
}

SteeringOutput Flee::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pHazardTarget->GetPosition(), elapsedSec);
}

SteeringOutput Evade::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	std::cout << "A static target can't be evaded\n";
	auto steering = SteeringOutput{};
	steering.IsValid = false;
	return steering;
}

SteeringOutput Evade::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	//auto steering = SteeringOutput{};
	//if (targetDistance > m_EvadeRadius) // If the distance's bigger the the EvadeRadius, stop moving
	//{
	//	steering.IsValid = false;
	//	return steering;
	//}

	const float targetDistance = pAgent->GetPosition().Distance(pMovingTarget->GetPosition()); // Calculate the distance between the agent and the target
	const float predictionTime = targetDistance / pAgent->GetCurrentSpeed(); // Calculate the prediction time
	const Vector2f predictedTarget = pMovingTarget->GetPosition() + pMovingTarget->GetMovementDirection() * pMovingTarget->GetCurrentSpeed() * predictionTime;
	return Flee::CalculateSteering(pAgent, predictedTarget, elapsedSec);
}

SteeringOutput Evade::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	if (pHazardTarget == nullptr)
	{
		steering.IsValid = false;
		return steering;
	}

	// If its a regular hazard, use the fixed target function
	auto* pMovingHazard = dynamic_cast<MovingHazard*>(pHazardTarget);
	if (pMovingHazard == nullptr)
		return CalculateSteering(pAgent, pHazardTarget->GetPosition(), elapsedSec);


	const float targetDistance = pAgent->GetPosition().Distance(pMovingHazard->GetPosition()); // Calculate the distance between the agent and the target
	const float predictionTime = targetDistance / pAgent->GetCurrentSpeed(); // Calculate the prediction time
	const Vector2f predictedTarget = pMovingHazard->GetPosition() + pMovingHazard->GetDirection() * pMovingHazard->GetSpeed() * predictionTime;
	return Flee::CalculateSteering(pAgent, predictedTarget, elapsedSec);
}

SteeringOutput Arrive::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	steering.MovementDirection = staticTarget - pAgent->GetPosition();
	float distance = pAgent->GetPosition().Distance(staticTarget);
	float speed = 1.f;

	if (distance < m_TargetRadius) // If the distance is inferior to the target radius, stop
	{
		steering.IsValid = false;
		return steering;
	}
	
	if (distance <= m_SlowRadius) // If the distance is inferior to the slow radius, go full speed
		speed = distance / m_SlowRadius;

	steering.MovementDirection.Normalize();
	steering.FacingDirection = steering.MovementDirection;
	steering.DesiredSpeed = MovementSpeed::Regular;
	steering.DesiredSpeedAmount = speed;
	return steering;
}

SteeringOutput Arrive::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pMovingTarget->GetPosition(), elapsedSec);
}

SteeringOutput Arrive::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pHazardTarget->GetPosition(), elapsedSec);
}

SteeringOutput Face::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	steering.FacingDirection = staticTarget - pAgent->GetPosition();
	steering.FacingDirection.Normalize();

	return steering;
}

SteeringOutput Face::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pMovingTarget->GetPosition(), elapsedSec);
}

SteeringOutput Face::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pHazardTarget->GetPosition(), elapsedSec);
}

SteeringOutput Wander::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	Vector2f offsetVector = pAgent->GetMovementDirection() * m_Offset; // Get the vector between the agent and the center of the circle
	auto target = pAgent->GetPosition() + offsetVector; // Add the agent position to the vector to get the center of the circle
	m_WanderAngle += m_AngleChange * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.f - 1.f); // Define m_WanderAngle between the max and min values of m_AngleChange
	Vector2f displacementVector = Vector2f(cos(m_WanderAngle), sin(m_WanderAngle)) * m_Radius; // Calculate the vector between the circle center and the final target (with the calculated angle)
	target += displacementVector; // Add the circle center position to the displacement vector to get the final target


	steering = Seek::CalculateSteering(pAgent, target, elapsedSec);
	return steering;
}

SteeringOutput Wander::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pMovingTarget->GetPosition(), elapsedSec);
}

SteeringOutput Wander::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pHazardTarget->GetPosition(), elapsedSec);
}

SteeringOutput WalkAwayCuriously::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	// If checking back, face the target
	if (m_CheckingBack)
	{
		// Get the direction towards the target
		auto desiredDirection = staticTarget - pAgent->GetPosition();
		desiredDirection.Normalize();

		// If the counter is finished, rotate back and switch behavior once fully turned
		auto finishingBehavior = false;
		if (m_Counter >= m_CurrentCooldown)
		{
			desiredDirection = pAgent->GetPosition() - staticTarget;
			desiredDirection.Normalize();
			finishingBehavior = true;
		}

		// Get the angle between the current and desired direction
		//auto angleBetweenDir = atan2f(desiredDirection.Determinant(pAgent->GetFacingDirection()), desiredDirection.Dot(pAgent->GetFacingDirection()));
		auto angleBetweenDir = atan2f(pAgent->GetFacingDirection().X, pAgent->GetFacingDirection().Y) - atan2f(desiredDirection.X, desiredDirection.Y);

		// And calculate how much can the agent turn in this frame
		m_CurrentTurningSpeed += m_TurningAccelaration * elapsedSec;
		if (m_CurrentTurningSpeed > m_MaxTurningSpeed) m_CurrentTurningSpeed = m_MaxTurningSpeed;
		auto angleChange = m_CurrentTurningSpeed * PI / 180.f * elapsedSec;

		// If the allowed turning ammount is superior to the desired ammount
		if (angleBetweenDir < 1.f * PI / 180.f && angleBetweenDir > -1.f * PI / 180.f)
		{
			// If its already the 2nd turn that finished, switch behavior
			if (finishingBehavior)
			{
				m_CheckingBack = false;
				m_Counter = 0.f;
				m_CurrentCooldown = m_MinMaxWalkAwayCooldown.X + (m_MinMaxWalkAwayCooldown.Y - m_MinMaxWalkAwayCooldown.X) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
				steering.FacingDirection = pAgent->GetFacingDirection();
				return steering;
			}

			// If it's the 1st turn, lock onto the target and start the counter
			m_Counter += elapsedSec;
			m_CurrentTurningSpeed = 0.f;
			steering.FacingDirection = staticTarget - pAgent->GetPosition();
			steering.FacingDirection.Normalize();
			return steering;
		}

		// If the allowed turning ammount isn't enough to face the target
		// Make sure the the angle change for this frame is in the right direction
		if (angleBetweenDir > 0.f)
		{
			if (angleChange >= angleBetweenDir) angleChange = angleBetweenDir;
		}
		else
		{
			if (angleChange >= angleBetweenDir * -1.f) angleChange = angleBetweenDir;
			else angleChange *= -1.f;
		}

		// And apply the angle change
		steering.FacingDirection.X = pAgent->GetFacingDirection().X * cos(angleChange) - pAgent->GetFacingDirection().Y * sin(angleChange);
		steering.FacingDirection.Y = pAgent->GetFacingDirection().X * sin(angleChange) + pAgent->GetFacingDirection().Y * cos(angleChange);
		return steering;
	}

	// If not checking back and if the counter has finished
	// Reset the counter and cooldown, and switch behavior
	m_Counter += elapsedSec;
	if (m_Counter >= m_CurrentCooldown)
	{
		m_CheckingBack = !m_CheckingBack;
		m_Counter = 0.f;

		if (m_CheckingBack)
			m_CurrentCooldown = m_MinMaxCheckBehindCooldown.X + (m_MinMaxCheckBehindCooldown.Y - m_MinMaxCheckBehindCooldown.X) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
		else
			m_CurrentCooldown = m_MinMaxWalkAwayCooldown.X + (m_MinMaxWalkAwayCooldown.Y - m_MinMaxWalkAwayCooldown.X) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
	
		steering.FacingDirection = pAgent->GetFacingDirection();
		return steering;
	}

	// If the counter's on-going, just flee slowly
	steering = Flee::CalculateSteering(pAgent, staticTarget, elapsedSec);
	steering.DesiredSpeed = MovementSpeed::Slow;
	return steering;
}

SteeringOutput WalkAwayCuriously::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pMovingTarget->GetPosition(), elapsedSec);
}

SteeringOutput WalkAwayCuriously::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	return CalculateSteering(pAgent, pHazardTarget->GetPosition(), elapsedSec);
}

SteeringOutput Surround::CalculateSteering(RegularAgent* pAgent, Vector2f staticTarget, float elapsedSec)
{
	std::cout << "A static target can't be surrounded, it needs to be an agent or a hazard\n";
	auto steering = SteeringOutput{};
	steering.IsValid = false;
	return steering;
}

SteeringOutput Surround::CalculateSteering(RegularAgent* pAgent, RegularAgent* pMovingTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	// Always face the target
	steering.FacingDirection = pMovingTarget->GetPosition() - pAgent->GetPosition();
	steering.FacingDirection.Normalize();

	// Move around the agent
	return steering;
}

SteeringOutput Surround::CalculateSteering(RegularAgent* pAgent, Hazard* pHazardTarget, float elapsedSec)
{
	auto steering = SteeringOutput{};

	// Always face the target
	steering.FacingDirection = pHazardTarget->GetPosition() - pAgent->GetPosition();
	steering.FacingDirection.Normalize();

	// Move around the hazard
	return steering;
}