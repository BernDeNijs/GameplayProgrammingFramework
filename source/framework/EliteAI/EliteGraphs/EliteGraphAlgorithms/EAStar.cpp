#include "stdafx.h"
#include "EAStar.h"

using namespace Elite;
AStar::AStar(Graph* pGraph, Heuristic hFunction)
	: m_pGraph(pGraph)
	, m_HeuristicFunction(hFunction)
{
}

std::vector<GraphNode*>AStar::FindPath(GraphNode* pStartNode, GraphNode* pEndNode) const
{
	std::vector<GraphNode*> path{};

	//create open and closed lists
	std::list<NodeRecord> openList{};
	std::list<NodeRecord> closedList{};
	

	
	//kick start the loop
	//-initialize first nodeRecord
	//NodeRecord currentNodeRecord{};
	//currentNodeRecord.pNode = pStartNode;
	//currentNodeRecord.pConnection = nullptr;
	//currentNodeRecord.costSoFar = 0;
	//currentNodeRecord.estimatedTotalCost = GetHeuristicCost(pStartNode,pEndNode);
	
	NodeRecord currentNodeRecord
	{ 
		pStartNode ,
		nullptr ,
		0,
		GetHeuristicCost(pStartNode,pEndNode) 
	};

	openList.push_back(currentNodeRecord);

	while (!openList.empty())
	{
		//get node with best cost from open list
		//currentNodeRecord = *std::min_element(openList.begin(), openList.end());
		auto currentNodeRecordIt = std::min_element(openList.begin(), openList.end());
		currentNodeRecord = *currentNodeRecordIt;


		if (currentNodeRecord.pNode == pEndNode)
		{
			break; //exit while loop if we found the endnode
		}
		
		for (GraphConnection* connection : m_pGraph->GetConnectionsFromNode(currentNodeRecord.pNode)) //check all connections
		{
			bool alreadyInList = false;
			GraphNode* nextNode = m_pGraph->GetNode(connection->GetToNodeId());
			const float newCost = connection->GetCost() + currentNodeRecord.costSoFar;
			//check if node leads to node already on closed list
			for (const NodeRecord & closedRecord : closedList)
			{
				if (closedRecord.pNode == nextNode)
				{
					if (closedRecord.costSoFar < newCost)
					{
						alreadyInList = true;
						break;
					}
					closedList.remove(closedRecord);
					break;
				}
			}
			if (alreadyInList) continue;

			//check if node leads to node already on open list
			for (const NodeRecord& openRecord : openList)
			{
				if (openRecord.pNode == nextNode)
				{
					if (openRecord.costSoFar < newCost)
					{
						alreadyInList = true;
						break;
					}
					openList.remove(openRecord);
					break;
				}
			}
			if (alreadyInList) continue;

			openList.push_back(
			NodeRecord
			{
				nextNode,
				connection,
				newCost,
				GetHeuristicCost(nextNode,pEndNode) + newCost
			}
			);
		}
		openList.remove(currentNodeRecord);
		closedList.push_back(currentNodeRecord);

	}

	while (currentNodeRecord.pNode != pStartNode)
	{
		path.push_back(currentNodeRecord.pNode);
		for (const NodeRecord& closedNodeRecord : closedList)
		{
			if (closedNodeRecord.pNode->GetId() == currentNodeRecord.pConnection->GetFromNodeId())
			{
				currentNodeRecord = closedNodeRecord;
				break;
			}
		}
	}
	path.push_back(pStartNode);
	std::ranges::reverse(path);

	return path;
}


float AStar::GetHeuristicCost(GraphNode* pStartNode, GraphNode* pEndNode) const
{
	const Vector2 toDestination = m_pGraph->GetNodePos(pEndNode->GetId()) - m_pGraph->GetNodePos(pStartNode->GetId());
	return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
}