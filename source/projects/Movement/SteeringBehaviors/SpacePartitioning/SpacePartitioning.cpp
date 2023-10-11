#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"
#include <algorithm>

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = Elite::Vector2{ left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	//Calculate bounds of a cell
	m_CellWidth = width / cols;
	m_CellHeight = height / rows;

	//TODO: create the cells
	m_Cells.reserve(m_NrOfRows * m_NrOfCols);
	for (size_t i = 0; i < m_NrOfRows * m_NrOfCols; i++)
	{
		m_Cells.emplace_back(Cell(static_cast<float>(i % m_NrOfCols) * m_CellWidth,static_cast<float>(i/ m_NrOfRows) * m_CellHeight,m_CellWidth,m_CellHeight));
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::AgentPositionChanged(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	// TODO: Check if the agent needs to be moved to another cell.
	// Use the calculated index for oldPos and currentPos for this
	int oldIdx = PositionToIndex(oldPos);
	int newIdx = PositionToIndex(agent->GetPosition());
	if (oldIdx == newIdx) return;

	m_Cells[oldIdx].agents.remove(agent);
	m_Cells[newIdx].agents.push_back(agent);
	//AddAgent(agent);
	
	

}

void CellSpace::RegisterNeighbors(SteeringAgent* pAgent, float neighborhoodRadius)
{
	//TODO: Register the neighbors for the provided agent
	//Only check the cells that are within the radius of the neighborhood

	m_NrOfNeighbors = 0;

	const auto agentPos = pAgent->GetPosition();
	Elite::Rect agentRect;
	agentRect.bottomLeft = Elite::Vector2{ agentPos.x - neighborhoodRadius/2.f ,agentPos.y - neighborhoodRadius/2.f };
	agentRect.height = neighborhoodRadius;
	agentRect.width = neighborhoodRadius;



	for (size_t i = 0; i < m_Cells.size(); i++)
	{
		if (Elite::IsOverlapping(agentRect,m_Cells[i].boundingBox))
		{
			/*DEBUGRENDERER2D->DrawCircle(m_Cells[i].boundingBox.bottomLeft, m_CellWidth/2.f ,{ 1.f,0.f,0.f }, DEBUGRENDERER2D->NextDepthSlice());*/

			for (const auto& agent : m_Cells[i].agents)
			{
				if (&agent == &pAgent) continue;
				const float distanceSquared = pAgent->GetPosition().DistanceSquared(agent->GetPosition());

				if (distanceSquared <= neighborhoodRadius * neighborhoodRadius)
				{
					m_Neighbors[m_NrOfNeighbors] = agent;
					++m_NrOfNeighbors;
				}
			}

		}
	}


}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	//TODO: Render the cells with the number of agents inside of it
	//TIP: use DEBUGRENDERER2D->DrawPolygon(...) and Cell::GetRectPoints())
	//TIP: use DEBUGRENDERER2D->DrawString(...) 
	for (int i = 0; i < m_Cells.size(); i++)
	{
		std::vector<Elite::Vector2> rectPoints = m_Cells[i].GetRectPoints();
		DEBUGRENDERER2D->DrawPolygon(&rectPoints[0], 4, Elite::Color(1.f, 1.f, 1.f), DEBUGRENDERER2D->NextDepthSlice());
		DEBUGRENDERER2D->DrawString(m_Cells[i].boundingBox.bottomLeft + Elite::Vector2{0.f,m_CellHeight}, std::to_string(static_cast<int>(m_Cells[i].agents.size())).c_str());
	}
	
	for (int i = 0; i < m_NrOfNeighbors; i++)
	{
		DEBUGRENDERER2D->DrawCircle(m_Neighbors[i]->GetPosition(), m_Neighbors[i]->GetRadius(), { 0.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());
	}

	
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	return  ( static_cast<int>(pos.y / m_CellHeight) * m_NrOfCols + static_cast<int>(pos.x / m_CellWidth));
}