#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "projects/Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"

using namespace Elite;
//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
{
	m_Agents.resize(m_FlockSize);
	m_Neighbors.resize(m_FlockSize - 1);
	//TODO: initialize the flock and the memory pool
	// Initialize steering
	m_pSeparationBehavior = new Separation(this);
	m_pCohesionBehavior = new Cohesion(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();



	//Spacial partitioning
	int cellRows = 20;
	int cellColumns = 20;
	int maxEntities = m_FlockSize;

	m_pCellSpace = new CellSpace(m_WorldSize, m_WorldSize, cellRows, cellColumns, maxEntities);
	//--


	m_pBlendedSteering = new BlendedSteering({
		{m_pSeparationBehavior,0.5f},
		{m_pCohesionBehavior,0.5f},
		{m_pVelMatchBehavior,0.5f},
		{m_pSeekBehavior,0.5f},
		{m_pWanderBehavior,0.5f},
		});

	m_pEvadeBehavior = new Evade();
	m_pEvadeBehavior->SetEvadeRange(20.f);

	m_pPrioritySteering = new PrioritySteering({
		{m_pEvadeBehavior},
		{m_pBlendedSteering}
		});


	for (int idx = 0; idx < m_FlockSize; idx++)
	{
		//m_Agents.emplace_back(new SteeringAgent());
		m_Agents[idx] = new SteeringAgent();
		m_Agents[idx]->SetMass(0.f);
		m_Agents[idx]->SetMaxLinearSpeed(20.0f);
		m_Agents[idx]->SetPosition(randomVector2(m_WorldSize).GetAbs());
		m_Agents[idx]->SetRotation(randomFloat());
		m_Agents[idx]->SetSteeringBehavior(m_pPrioritySteering);
		/*m_Agents[idx]->SetBodyColor(Color{ 1.f,1.f,0.f });*/
		m_Agents[idx]->SetAutoOrient(true);

		m_pCellSpace->AddAgent(m_Agents[idx]);		
	}

	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetMass(5.f);
	m_pAgentToEvade->SetMaxLinearSpeed(15.0f);
	m_pAgentToEvade->SetPosition({ (m_WorldSize) / 2.f ,(m_WorldSize) / 2.f });
	m_pAgentToEvade->SetRotation(randomFloat());
	m_pAgentToEvade->SetSteeringBehavior(m_pWanderBehavior);
	m_pAgentToEvade->SetBodyColor(Color{ 1.f,0.f,0.f });
	m_pAgentToEvade->SetAutoOrient(true);



}

Flock::~Flock()
{
	//TODO: Cleanup any additional data

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pCellSpace);

	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	SAFE_DELETE(m_pAgentToEvade);
	m_Agents.clear();
	m_Neighbors.clear();
}

void Flock::Update(float deltaT)
{



	m_pEvadeBehavior->SetTarget(TargetData{ 
		m_pAgentToEvade->GetPosition(),
		m_pAgentToEvade->GetRotation(),
		m_pAgentToEvade->GetLinearVelocity(),
		m_pAgentToEvade->GetAngularVelocity() 
		});

	for (const auto& agent : m_Agents)
	{
		const Elite::Vector2 oldPos = agent->GetPosition();
		RegisterNeighbors(agent);
		agent->Update(deltaT);
		
		if (m_SpacialPartitioning)
		{
			m_pCellSpace->AgentPositionChanged(agent, oldPos);
		}
		agent->TrimToWorld(m_WorldSize);
	}
	m_pAgentToEvade->Update(deltaT);
	m_pAgentToEvade->TrimToWorld(m_WorldSize);


}

void Flock::Render(float deltaT)
{
	for (const auto& pAgent : m_Agents)
	{
		if (pAgent != nullptr)
		{
			pAgent->Render(deltaT);
		}
	}

	if (m_pAgentToEvade)
	{
		m_pAgentToEvade->Render(deltaT);
	}
	
	if (m_RenderNeighboorhood) 
		RenderNeighborhood();
	else
	{
		m_Agents[0]->SetRenderBehavior(false);
	}

	m_pCellSpace->RenderCells();
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	//TODO: implement ImGUI checkboxes for debug rendering here

	//ImGui::Checkbox("Debug Rendering", &m_RenderNeighboorhood);
	ImGui::Checkbox("Debug Rendering", &m_RenderNeighboorhood);
	ImGui::Checkbox("Spatial Partitioning", &m_SpacialPartitioning);

	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	//TODO: implement ImGUI sliders for steering behavior weights here
	// 
	ImGui::SliderFloat("Seperation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("VelocityMatch", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");
	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RenderNeighborhood()
{
	// TODO: Implement
	// Register the neighbors for the first agent in the flock
	RegisterNeighbors(m_Agents[0]);
	m_Agents[0]->SetRenderBehavior(true);
	// DebugRender the neighbors in the memory pool
	DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), m_NeighborhoodRadius, { 1.f,1.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());
	for (int i = 0; i < m_NrOfNeighbors; i++)
	{
		DEBUGRENDERER2D->DrawSolidCircle(m_Neighbors[i]->GetPosition(), m_Neighbors[i]->GetRadius(), m_Neighbors[i]->GetLinearVelocity().GetNormalized(), {0.f,1.f,0.f});
	}
}


void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	

	if (m_SpacialPartitioning)
	{
		m_pCellSpace->RegisterNeighbors(pAgent, m_NeighborhoodRadius);
		m_Neighbors = m_pCellSpace->GetNeighbors();
		m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();

	}
	else
	{
		m_NrOfNeighbors = 0;
		for (const auto& agent : m_Agents)
		{
			if (&agent == &pAgent) continue; //changed
			const float distanceSquared = pAgent->GetPosition().DistanceSquared(agent->GetPosition());

			if (distanceSquared <= m_NeighborhoodRadius * m_NeighborhoodRadius)
			{
				m_Neighbors[m_NrOfNeighbors] = agent;
				++m_NrOfNeighbors;
			}
		}
	}
	
}

int Flock::GetNrOfNeighbors() const
{
	if (m_SpacialPartitioning) return m_pCellSpace->GetNrOfNeighbors();
	return m_NrOfNeighbors;
}

const std::vector<SteeringAgent*>& Flock::GetNeighbors() const
{
	if (m_SpacialPartitioning) return m_pCellSpace->GetNeighbors();
	return m_Neighbors; 
}

Vector2 Flock::GetAverageNeighborPos() const
{
	Vector2 avgPosition = Elite::ZeroVector2;

	for (int i = 0; i < m_NrOfNeighbors; i++)
	{
		avgPosition += m_Neighbors[i]->GetPosition();
	}

	avgPosition /= static_cast<float>( m_NrOfNeighbors);

	return avgPosition;
}

Vector2 Flock::GetAverageNeighborVelocity() const
{
	Vector2 avgVelocity = Elite::ZeroVector2;

	for (int i = 0; i < m_NrOfNeighbors; i++)
	{
		avgVelocity += m_Neighbors[i]->GetLinearVelocity();
	}
	avgVelocity /= static_cast<float>(m_NrOfNeighbors);
	

	return avgVelocity;
}
void Flock::SetTarget_Seek(const TargetData& target)
{
	m_pSeekBehavior->SetTarget(target);
	// TODO: Implement
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
