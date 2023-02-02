#pragma once
#include "CommonHeader.h"
#include <vector>

class SteeringBehavior;
class Hazard;
class FlowField;
class Wall;

enum class Emotion
{
	Calm = 0,
	Fright = 1,
	Panic = 2
};

class RegularAgent
{
public:
	RegularAgent(Vector2f initialPos, Vector2f initialDir, OCEANProfile emotionalProfile, FlowField* pExitFlowField,
		std::vector<FlowField*> pRoomsFlowFields, std::vector<Quad*> pRoomsQuads, std::vector<Wall*>* pWalls, std::vector<RegularAgent*>* pAgents);
	virtual ~RegularAgent();

	RegularAgent(const RegularAgent&) = delete;
	RegularAgent(RegularAgent&&) = delete;
	RegularAgent& operator=(const RegularAgent&) = delete;
	RegularAgent& operator=(const RegularAgent&&) = delete;

	void DrawBackground();
	virtual void DrawForward(bool isPicked);
	virtual void DrawHUD();
	virtual void Update(float elapsedSec);

	virtual void ReactToHazardRange(Hazard* pHazard, float elapsedSec);
	virtual bool GetIsAuthority() const { return false; }
	virtual bool GetIsAlert() const { return false; }
	void Escape();


	// Setters & Getters
	void SetTarget(Vector2f newTarget) { m_TargetPos = newTarget; }
	Vector2f GetTarget() const { return m_TargetPos; }

	void SetPosition(Vector2f newPosition) { m_Position = newPosition; }
	Vector2f GetPosition() const  { return m_Position; }

	Vector2f GetPositionLastFrame() const { return m_PositionLastFrame; }
	Vector2f GetMovementDirection() const { return m_MovementDirection; }
	Vector2f GetFacingDirection() const { return m_FacingDirection; }
	float GetCurrentSpeed() const { return m_CurrentSpeed; }
	float GetAgentDiameter() const {return m_AgentDiameter;}
	bool GetHasEscaped() const { return m_HasEscaped; }
	Emotion GetCurrentEmotion() const { return m_CurrentEmotion; }
	bool GetCurrentlySeekingNewRoom() const { return m_CurrentlySeekingNewRoom; }
	OCEANProfile GetEmotionalProfile() const { return m_EmotionalProfile; }
	

protected:
	void AddAgentSeparationToSteering(SteeringOutput& steeringOutput, float elapsedSec, float intensity);
	void AddWallSeparationToSteering(SteeringOutput& steeringOutput, float elapsedSec, float intensity);
	virtual void GetEmotionallyInfected(float elapsedSec);

	void ResetAllBehaviors();

	void PickNewRoomToSeek(bool avoidKnownRooms = true);

	bool m_HasEscaped;
	
	FlowField* m_pExitFlowField;
	std::vector <FlowField*> m_pRoomsFlowFields;
	std::vector<Quad*> m_pRoomsQuads;
	std::vector<Wall*>* m_pWalls;
	std::vector<RegularAgent*>* m_pAgents;

	Vector2f m_Position;
	Vector2f m_PositionLastFrame;
	Vector2f m_MovementDirection;
	Vector2f m_FacingDirection;
	Vector2f m_TargetPos;
	float m_CurrentSpeed;
	const float m_RegularSpeed;
	const float m_SprintSpeed;
	const float m_SlowSpeed;
	const float m_AgentDiameter;
	const OCEANProfile m_EmotionalProfile;
	const float m_OtherAgentAwarenessRadius;

	bool m_CurrentlySeekingNewRoom;
	int m_CurrentRoomIdx;
	int m_LastRoomBeforeCurrentIdx;
	int m_CurrentlySeekedRoomIdx;
	const Vector2f m_RoomExploringMinMaxTime;
	float m_CurrentRoomExploringTime;
	float m_RoomExploringCounter;
	std::vector<std::pair<bool, float>> m_Rooms; // if found and time since left
	const float m_RoomMemoryTime;


	const float m_CalmSpreadRate;
	const float m_FrightSpreadRate;
	const float m_PanicSpreadRate;
	const float m_CalmAttenuationRate;
	const float m_FrightAttenuationRate;
	const float m_PanicAttenuationRate;
	const float m_MaxEmotionalInfectionDist;

	float m_PercievedCalm;
	float m_PercievedFright;
	float m_PercievedPanic;

	Emotion m_CurrentEmotion;
	Hazard* m_pLastHazard;

	bool m_OnEmotionalCooldown;
	const float m_EmotionalCooldownTime;
	float m_EmotionalCooldownCounter;

	SteeringBehavior* m_pSeekBehavior;
	SteeringBehavior* m_pFleeBehavior;
	SteeringBehavior* m_pWanderBehavior;
};

