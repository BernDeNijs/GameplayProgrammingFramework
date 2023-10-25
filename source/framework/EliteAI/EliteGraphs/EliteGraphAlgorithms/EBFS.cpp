#include "stdafx.h"
#include "EBFS.h"

#include "../EliteGraph/EGraph.h"
#include "../EliteGraph/EGraphNode.h"
#include "../EliteGraph/EGraphConnection.h"

using namespace Elite;

BFS::BFS(Graph* pGraph)
	: m_pGraph(pGraph)
{
}

//Breath First Search Algorithm searches for a path from the startNode to the destinationNode
std::vector<GraphNode*> BFS::FindPath(GraphNode* pStartNode, GraphNode* pDestinationNode)
{
	std::vector<GraphNode*> path{};
	//create open and closed lists
	std::queue<GraphNode*> openList{};
	std::map<GraphNode*, GraphNode*> closedList{};

	//add startNode too list
	openList.push(pStartNode);

	//go over nodes
	while (!openList.empty())
	{
		
		GraphNode* currentNode = openList.front();	//get current node
		openList.pop();								//remove current node from list

		if (currentNode == pDestinationNode)		//stop if we found the destination
		{
			break;
		}

		for (const GraphConnection* connection : m_pGraph->GetConnectionsFromNode(currentNode)) //search all neighbors
		{
			GraphNode* nextNode = m_pGraph->GetNode(connection->GetToNodeId());

			if (closedList.contains(nextNode))
			{
				continue;
			}
			//add neighbors if not previously added
			openList.push(nextNode);
			closedList[nextNode] = currentNode;
		} 
	}

	if (!closedList.contains(pDestinationNode)) //if we never found the destination return an empty path
	{
		return path;
	}

	//backtracking to build path
	GraphNode* currentNode = pDestinationNode;
	while (currentNode != pStartNode)
	{
		path.push_back(currentNode);
		currentNode = closedList[currentNode]; //get node connected to current node
	}
	path.push_back(pStartNode); //add startnode to the path

	//std::reverse(path.begin(), path.end()); //get path back in right order
	std::ranges::reverse(path);	//get path back in right order / used ranges to not bother with begin & end

	return path;
}