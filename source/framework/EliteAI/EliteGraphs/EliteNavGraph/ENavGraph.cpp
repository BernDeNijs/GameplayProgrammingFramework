#include "stdafx.h"
#include "ENavGraph.h"
#include "../EliteGraph/EGraphNode.h"
#include "../EliteGraph/EGraphConnection.h"
#include "../EliteGraphNodeFactory/EGraphNodeFactory.h"
//#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

NavGraph::NavGraph(const std::vector<Polygon>& colliderShapes, float widthWorld, float heightWorld, float playerRadius = 1.0f) :
	Graph(false, new GraphNodeFactoryTemplate<NavGraphNode>()),
	m_pNavMeshPolygon(nullptr)
{
	float const halfWidth = widthWorld / 2.0f;
	float const halfHeight = heightWorld / 2.0f;
	std::list<Vector2> baseBox
	{ { -halfWidth, halfHeight },{ -halfWidth, -halfHeight },{ halfWidth, -halfHeight },{ halfWidth, halfHeight } };

	m_pNavMeshPolygon = new Polygon(baseBox); // Create copy on heap

	//Store all children
	for (auto p : colliderShapes)
	{
		p.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(p);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

NavGraph::NavGraph(const NavGraph& other): Graph(other)
{
}

NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon;
	m_pNavMeshPolygon = nullptr;
}

std::shared_ptr<NavGraph> NavGraph::Clone()
{
	return std::shared_ptr<NavGraph>(new NavGraph(*this));
}

int NavGraph::GetNodeIdFromLineIndex(int lineIdx) const
{

	for (auto& pNode : m_pNodes)
	{
		if (reinterpret_cast<NavGraphNode*>(pNode)->GetLineIndex() == lineIdx)
		{
			return pNode->GetId();
		}
	}

	return invalid_node_id;
}

Elite::Polygon* NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create a node on the center of each edge
	//std::vector<Line*> lines = m_pNavMeshPolygon->GetLines();
	//for (const Line* line : lines)
	for (const Line* line : m_pNavMeshPolygon->GetLines())
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() != 2) continue;
		AddNode(new NavGraphNode(line->index,(line->p1 + line->p2) * 0.5f));
	}

	//2  Now that every node is created, connect the nodes that share the same triangle (for each triangle, ... )
	//std::vector<Triangle*> triangles = m_pNavMeshPolygon->GetTriangles();
	for (const Triangle* triangle : m_pNavMeshPolygon->GetTriangles())
	{
		std::vector<int> tempIds;
		for (int lineIdx : triangle->metaData.IndexLines)
		{
			int nodeId = GetNodeIdFromLineIndex(lineIdx);
			//check for invalid
			if (nodeId == -1) continue;
			tempIds.push_back(nodeId);
		}
		
		if (tempIds.size() == 2)
		{
			AddConnection(new GraphConnection(
				tempIds[0], 
				tempIds[1]
			));
		}
		else if (tempIds.size() == 3)
		{
			AddConnection(new GraphConnection(
				tempIds[0],
				tempIds[1]
			));
			AddConnection(new GraphConnection(
				tempIds[1],
				tempIds[2]
			));
			AddConnection(new GraphConnection(
				tempIds[0],
				tempIds[2])
			);
		}
	}
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistances();
}

