#pragma once
#include "RegularAgent.h"

enum class AlertState
{
	NotAlert = 0,
	SeekingAgentsInRoom = 1,
	ChangingRoom = 2,
	LeavingScene = 3
};

class ThirdPartyAgent  final : public RegularAgent
{
public:
	ThirdPartyAgent(Vector2f initialPos, Vector2f initialDir, FlowField* pExitFlowField,
		std::vector<FlowField*> pRoomsFlowFields, std::vector<Quad*> pRoomsQuads, std::vector<Wall*>* pWalls, std::vector<RegularAgent*>* pAgents);
	~ThirdPartyAgent();

	ThirdPartyAgent(const ThirdPartyAgent&) = delete;
	ThirdPartyAgent(ThirdPartyAgent&&) = delete;
	ThirdPartyAgent& operator=(const ThirdPartyAgent&) = delete;
	ThirdPartyAgent& operator=(const ThirdPartyAgent&&) = delete;

	void DrawForward(bool isPicked) override;
	void DrawHUD() override;
	void Update(float elapsedSec) override;

	void ReactToHazardRange(Hazard* pHazard, float elapsedSec) override;

	bool GetIsAuthority() const override { return true; }
	bool GetIsAlert() const override { return m_AlertState != AlertState::NotAlert; }

protected:
	void GetEmotionallyInfected(float elapsedSec) override;

private:
	void BecomeAlert();
	int GetClosestUncheckedRoomIdx();

	AlertState m_AlertState;


	bool m_StartedCheckingRooms = false;
	int m_TargetRoomToCheck = -1;
};

