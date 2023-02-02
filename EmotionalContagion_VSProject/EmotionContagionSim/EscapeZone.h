#pragma once
#include "CommonHeader.h"
#include <vector>

class RegularAgent;

class EscapeZone
{
public:
	EscapeZone(Vector2f initialPos, std::vector<RegularAgent*> pAgents, Vector2f size = Vector2f(20.f, 20.f));
	virtual ~EscapeZone();

	EscapeZone(const EscapeZone&) = delete;
	EscapeZone(EscapeZone&&) = delete;
	EscapeZone& operator=(const EscapeZone&) = delete;
	EscapeZone& operator=(const EscapeZone&&) = delete;

	void Draw(bool exitSelected);
	void Update();

	void ResetAgents(std::vector<RegularAgent*> pAgents);

	Vector2f GetPosition() const { return m_Position; }

private:
	Vector2f m_Position;
	Vector2f m_Size;
	std::vector<RegularAgent*> m_pAgents;
};

