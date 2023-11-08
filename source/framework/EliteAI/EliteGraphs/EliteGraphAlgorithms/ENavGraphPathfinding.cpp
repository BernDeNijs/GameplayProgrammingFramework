#include "stdafx.h"

#include "ENavGraphPathfinding.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAStar.h"
#include "framework/EliteAI/EliteGraphs/EliteNavGraph/ENavGraph.h"

using namespace Elite;

std::vector<Vector2> NavMeshPathfinding::FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
{
	//Create the path to return
	std::vector<Vector2> finalPath{};

	//Get the startTriangle and endTriangle

	//If we don't have a valid startTriangle or endTriangle -> return empty path
	//If the startTriangle and endTriangle are the same -> return straight line path
	
	//=> Start looking for a path
	//Clone the graph (returns shared_ptr!)

	//Create extra node for the Start Node (Agent's position) and add it to the graph. 
	//Make connections between the Start Node and the startTriangle nodes.

	//Create extra node for the End Node (endpos) and add it to the graph. 
	//Make connections between the End Node and the endTriangle nodes.

	//Run AStar on the new graph

	//Run optimiser on new graph, MAKE SURE the AStar path is working properly before starting the following section:
	//m_Portals = SSFA::FindPortals(nodes, m_pNavGraph->GetNavMeshPolygon());
	//finalPath = SSFA::OptimizePortals(m_Portals);

	return finalPath;
}
