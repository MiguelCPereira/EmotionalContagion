#pragma once
#include "CommonHeader.h"
#include <vector>

class Wall;
class Hazard;

struct FlowFieldCell
{
	FlowFieldCell(Quad proportions, std::vector<FlowFieldCell*> neighborCells)
		: Position(proportions.BottomLeft + proportions.Size / 2.f)
		, Proportions(proportions)
		, Cost()
		, NeighborCells(neighborCells)
	{}

	FlowFieldCell(Quad proportions)
		: Position(proportions.BottomLeft + proportions.Size / 2.f)
		, Proportions(proportions)
		, Cost()
		, NeighborCells()
	{}

	const Vector2f Position;
	const Quad Proportions;
	int Cost;

	std::vector<FlowFieldCell*> NeighborCells;
};

class FlowField
{
public:
	FlowField(Vector2f sceneSize, Vector2f cellsAmount, std::vector<Wall*> pMapWalls, std::vector<Hazard*> pMapHazards, Vector2f targetPos, Vector2f sceneBotLeft = Vector2f());
	~FlowField();

	FlowField(const FlowField&) = delete;
	FlowField(FlowField&&) = delete;
	FlowField& operator=(const FlowField&) = delete;
	FlowField& operator=(const FlowField&&) = delete;

	void Draw();

	Vector2f GetNextCellPosition(Vector2f agentPosition) const;

private:
	const Vector2f m_CellsAmount;
	Vector2f m_CellSize;
	std::vector<std::vector<FlowFieldCell*>> m_pCells;
	const std::vector<Wall*> m_pMapWalls;
	const std::vector<Hazard*> m_pMapHazards;
	const Vector2f m_TargetPosition;
	int m_TargetCellIdxX;
	int m_TargetCellIdxY;
	FlowFieldCell* m_pTargetCell;
	const int m_MinHazardCost;
	const int m_MaxHazardCost;
	const int m_InvalidCost;
};

