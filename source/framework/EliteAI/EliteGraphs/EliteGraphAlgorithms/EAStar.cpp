#include "stdafx.h"
#include "EAStar.h"

using namespace Elite;
AStar::AStar(Graph* pGraph, Heuristic hFunction)
	: m_pGraph(pGraph)
	, m_HeuristicFunction(hFunction)
{
}

std::vector<GraphNode*>AStar::FindPath(GraphNode* pStartNode, GraphNode* pGoalNode)
{
	std::vector<GraphNode*> path{};
	
	return path;
}


float AStar::GetHeuristicCost(GraphNode* pStartNode, GraphNode* pEndNode) const
{
	Vector2 toDestination = m_pGraph->GetNodePos(pEndNode->GetId()) - m_pGraph->GetNodePos(pStartNode->GetId());
	return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
}