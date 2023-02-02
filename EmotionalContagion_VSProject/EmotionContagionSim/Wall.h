#pragma once
#include "CommonHeader.h"
#include <vector>

class RegularAgent;

class Wall
{
public:
	Wall(Vector2f initialPos, std::vector<RegularAgent*> pAgents, Vector2f size = Vector2f(150.f, 30.f), bool visible = true);
	virtual ~Wall();

	Wall(const Wall&) = delete;
	Wall(Wall&&) = delete;
	Wall& operator=(const Wall&) = delete;
	Wall& operator=(const Wall&&) = delete;

	void Draw();
	void Update();

	void ResetAgents(std::vector<RegularAgent*> pAgents);

	Vector2f GetPosition() const { return m_Position; }
	Vector2f GetBottomLeft() const { return m_Position - m_Size / 2.f; }
	Vector2f GetSize() const { return m_Size; }

private:
	Vector2f m_Position;
	Vector2f m_Size;
	std::vector<RegularAgent*> m_pAgents;
	bool m_Visible;
	const float m_BounceBackAmount;
};

