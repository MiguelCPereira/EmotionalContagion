#include "FlowField.h"
#include "GameEngine.h"
#include "Wall.h"
#include "Hazard.h"
#include <list>

FlowField::FlowField(Vector2f sceneSize, Vector2f cellsAmount, std::vector<Wall*> pMapWalls, std::vector<Hazard*> pMapHazards, Vector2f targetPos, Vector2f sceneBotLeft)
	: m_CellsAmount(cellsAmount)
	, m_pMapWalls(pMapWalls)
	, m_pMapHazards(pMapHazards)
	, m_TargetPosition(targetPos)
	, m_TargetCellIdxX(-1)
	, m_TargetCellIdxY(-1)
	, m_MinHazardCost(cellsAmount.X * cellsAmount.Y)
	, m_MaxHazardCost(cellsAmount.X * cellsAmount.Y * 2)
	, m_InvalidCost(cellsAmount.X* cellsAmount.Y * 3)
{
	m_CellSize = Vector2f((sceneSize.X - 9.f) / cellsAmount.X, (sceneSize.Y - 9.f) / cellsAmount.Y);

	// Fill up the cells vector, keeping each row as a seperate vector for ease of use
	for (auto y = 0; y < cellsAmount.Y; y++)
	{
		std::vector<FlowFieldCell*> rowVector;
		const auto cellBottom = sceneBotLeft.Y + m_CellSize.Y * float(y);
		for (auto x = 0; x < cellsAmount.X; x++)
		{
			auto* pCell = new FlowFieldCell(Quad(Vector2f(sceneBotLeft.X + m_CellSize.X * float(x), cellBottom), m_CellSize));

			if (m_pTargetCell == nullptr && m_TargetPosition.IsWithinRectArea(Vector2f(sceneBotLeft.X + m_CellSize.X * float(x), sceneBotLeft.Y + m_CellSize.Y * float(y)), m_CellSize))
			{
				m_pTargetCell = pCell;
				m_TargetCellIdxX = x;
				m_TargetCellIdxY = y;
			}

			// If the cell collides with a wall, set its cost to invalid
			const Quad cellQuad(Vector2f(sceneBotLeft.X + m_CellSize.X * float(x), sceneBotLeft.Y + m_CellSize.Y * float(y)), m_CellSize);
			for (auto* pWall : m_pMapWalls)
			{
				if (cellQuad.IsOverlappingOtherRect(Quad(pWall->GetBottomLeft(), pWall->GetSize())))
				{
					pCell->Cost = m_InvalidCost;
					break;
				}
				else pCell->Cost = m_InvalidCost + 1;
			}

			// If the cell collides with a hazard, set its cost to invalid
			if (pCell->Cost != m_InvalidCost)
			{
				for (auto* pHazard : m_pMapHazards)
				{
					if (cellQuad.IsOverlappingCircle(pHazard->GetPosition(), pHazard->GetEffectRange() / 2.f))
					{
						const auto cellCenter = cellQuad.BottomLeft + cellQuad.Size / 2.f;
						const auto distance = cellCenter.Distance(pHazard->GetPosition());
						auto distPerc = 0;
						if (distance > 0.f) distPerc = distance / (pHazard->GetEffectRange() / 2.f);

						if (distPerc < 0.f) distPerc = 0.f;
						else if(distPerc > 1.f) distPerc = 1.f;

						pCell->Cost = m_MinHazardCost + (m_MaxHazardCost - m_MinHazardCost) * (1.f - distPerc);
						break;
					}
				}
			}

			rowVector.push_back(pCell);
		}
		m_pCells.push_back(rowVector);
	}

	// Set the neighbors of each cell
	for (auto y = 0; y < m_pCells.size(); y++)
	{
		const auto topBlocked = y == m_pCells.size() - 1;
		const auto botBlocked = y == 0 || y == m_pCells.size() - 1;

		for (auto x = 0; x < m_pCells[y].size(); x++)
		{
			const auto leftBlocked = x == 0;
			const auto rightBlocked = x == m_pCells[y].size() - 1;

			// Perpendicular neighbors
			if (!topBlocked)
				m_pCells[y][x]->NeighborCells.push_back(m_pCells[y + 1][x]);
			if (!botBlocked)
				m_pCells[y][x]->NeighborCells.push_back(m_pCells[y - 1][x]);
			if (!leftBlocked)
				m_pCells[y][x]->NeighborCells.push_back(m_pCells[y][x - 1]);
			if (!rightBlocked)
				m_pCells[y][x]->NeighborCells.push_back(m_pCells[y][x + 1]);

			// Diagonal neighbors
			if(!leftBlocked && !topBlocked)
				m_pCells[y][x]->NeighborCells.push_back(m_pCells[y + 1][x - 1]);
			if (!leftBlocked && !botBlocked)
				m_pCells[y][x]->NeighborCells.push_back(m_pCells[y - 1][x - 1]);
			if (!rightBlocked && !topBlocked)
				m_pCells[y][x]->NeighborCells.push_back(m_pCells[y + 1][x + 1]);
			if (!rightBlocked && !botBlocked)
				m_pCells[y][x]->NeighborCells.push_back(m_pCells[y - 1][x + 1]);

		}
	}

	// Set the cost of each cell
	// Inspired by https://leifnode.com/2013/12/flow-field-pathfinding/

	std::list<FlowFieldCell*> openList;
	m_pTargetCell->Cost = 0;
	openList.push_back(m_pTargetCell);

	while (openList.empty() == false)
	{
		//Get the next node in the list
		auto* currentCell = openList.front();
		openList.pop_front();

		//Get the neighbors of the current node
		auto neighbors = currentCell->NeighborCells;
		int neighborCount = neighbors.size();

		for (int i = 0; i < neighborCount; i++)
		{   
			// Calculate the new cost of the neighbor node
			auto endNodeCost = currentCell->Cost + 1;
			if (neighbors[i]->Cost == m_InvalidCost) endNodeCost = m_InvalidCost;
			if (neighbors[i]->Cost >= m_MinHazardCost && neighbors[i]->Cost <= m_MaxHazardCost) endNodeCost = neighbors[i]->Cost;

			// If this path is cheaper than the one the neighbor has saved already
			if (endNodeCost < neighbors[i]->Cost)
			{
				// If the neighbor's not in the list, add them
				if(std::find(openList.begin(), openList.end(), neighbors[i]) == std::end(openList))
					openList.push_back(neighbors[i]);

				// And replace their cost
				neighbors[i]->Cost = endNodeCost;
			}
		}
	}
}

FlowField::~FlowField()
{
	for (const auto& rowVector : m_pCells)
	{
		for (auto* pCell : rowVector)
		{
			delete pCell;
			pCell = nullptr;
		}
	}
	m_pCells.clear();
}

void FlowField::Draw()
{
	GAME_ENGINE->SetColor(RGB(255 * 0.5, 255 * 0.5, 255 * 0.5));

	for (const auto& rowVector : m_pCells)
	{
		for (auto* pCell : rowVector)
		{
			wstring costText = L"X";
			if (pCell->Cost != m_InvalidCost)
			{
				if (pCell->Cost < m_MinHazardCost) costText = std::to_wstring(pCell->Cost);
				else costText = L" !";
			}

			GAME_ENGINE->DrawRect(int(pCell->Proportions.BottomLeft.X), int(pCell->Proportions.BottomLeft.Y), int(pCell->Proportions.Size.X), int(pCell->Proportions.Size.Y));
			GAME_ENGINE->DrawString(costText, pCell->Proportions.BottomLeft.X, pCell->Proportions.BottomLeft.Y);
		}
	}
}

Vector2f FlowField::GetNextCellPosition(Vector2f agentPosition) const
{
	FlowFieldCell* pAgentCell = nullptr;
	FlowFieldCell* pTargetCell = nullptr;

	// Find which tile the agent is standing on
	//for (int i = 0; i < m_pCells.size(); i++)
	for (const auto& rowVector : m_pCells)
	{
		//const auto& rowVector = m_pCells[i];
		if (pAgentCell != nullptr) break;

		for (auto* pCell : rowVector)
		{
			if (agentPosition.IsWithinRectArea(pCell->Proportions.BottomLeft, pCell->Proportions.Size))
			{
				pAgentCell = pCell;
				break;
			}
		}
	}

	// If the agentCell is nullptr, early return
	// Something went wrong, the agent should always be on the map
	if (pAgentCell == nullptr)
	{
		std::cout << "Something went wrong, the agent isn't standing on any tile\n";
		return Vector2f();
	}

	// Get the cheapest neighboring cell
	for (auto* pCell : pAgentCell->NeighborCells)
	{
		if (pTargetCell == nullptr || pTargetCell->Cost > pCell->Cost)
			pTargetCell = pCell;
	}

	// If the targetCell is nullptr, early return
	// Something went wrong, there should always be a target cell
	if (pTargetCell == nullptr)
	{
		std::cout << "Something went wrong, there should always be a target cell\n";
		return Vector2f();
	}

	return pTargetCell->Position;
}