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
	const Triangle* startTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
	if (startTriangle == nullptr) return finalPath;
	const Triangle* endTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);
	if (endTriangle == nullptr) return finalPath;
	

	//If the startTriangle and endTriangle are the same -> return straight line path
	if (startTriangle == endTriangle)
	{
		finalPath.push_back(startPos);
		finalPath.push_back(endPos);
		return finalPath;
	}
		
	//=> Start looking for a path
	//Clone the graph (returns shared_ptr!)
	const std::shared_ptr<NavGraph> graphCopy = pNavGraph->Clone();

	//Create extra node for the Start Node (Agent's position) and add it to the graph. 
	const auto startNode = new NavGraphNode(-1,startPos);
	const int startNodeId = graphCopy->AddNode(startNode);
	
	//Make connections between the Start Node and the startTriangle nodes.
	for (const auto lineIdx : startTriangle->metaData.IndexLines)
	{
		const int nodeId = graphCopy->GetNodeIdFromLineIndex(lineIdx);
		if (nodeId == -1) continue;		
		graphCopy->AddConnection(new GraphConnection(startNodeId,nodeId,
			Elite::Distance(startPos,graphCopy->GetNodePos(nodeId))));
	}
	
	//Create extra node for the End Node (endpos) and add it to the graph. 
	const auto endNode = new NavGraphNode(-1, endPos);
	const int endNodeId = graphCopy->AddNode(endNode);
	//Make connections between the End Node and the endTriangle nodes.
	for (const auto lineIdx : endTriangle->metaData.IndexLines)
	{
		const int nodeId = graphCopy->GetNodeIdFromLineIndex(lineIdx);
		if (nodeId == -1) continue;
		graphCopy->AddConnection(new GraphConnection(endNodeId, nodeId,
			Elite::Distance(endPos, graphCopy->GetNodePos(nodeId))));
	}
	//Run AStar on the new graph
	const auto pathfinder = AStar(graphCopy.get(), HeuristicFunctions::Euclidean);
	const auto nodes = pathfinder.FindPath(startNode, endNode);

	////-- tempcode for A* Test
	//debugNodePositions.clear();
	//for (const auto& node : tempPath)
	//{
	//	finalPath.push_back(node->GetPosition());
	//	debugNodePositions.push_back(node->GetPosition());
	//}
	////-- tempcode end


	//Run optimiser on new graph, MAKE SURE the AStar path is working properly before starting the following section:
	auto portals = SSFA::FindPortals(nodes, pNavGraph->GetNavMeshPolygon());
	finalPath = SSFA::OptimizePortals(portals);

	return finalPath;
}
