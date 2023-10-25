#pragma once
// Here we determine which application is currently active
// Create the define here using the "ActiveApp_..." convention and set up the correct include and typedef in the #ifdef below.

//#define ActiveApp_Sandbox
//#define ActiveApp_SteeringBehaviors
//#define ActiveApp_CombinedSteering
//#define ActiveApp_Flocking
#define ActiveApp_GraphTheory
//#define ActiveApp_PathfindingAStar

//---------- Registered Applications -----------
#ifdef ActiveApp_Sandbox
#include "projects/Movement/Sandbox/App_Sandbox.h"
typedef App_Sandbox CurrentApp;
#endif

#ifdef ActiveApp_SteeringBehaviors
#include "projects/Movement/SteeringBehaviors/Steering/App_SteeringBehaviors.h"
typedef App_SteeringBehaviors CurrentApp;
#endif

#ifdef  ActiveApp_CombinedSteering
#include "projects/Movement/SteeringBehaviors/CombinedSteering/App_CombinedSteering.h"
typedef App_CombinedSteering CurrentApp;
#endif

#ifdef ActiveApp_Flocking
#include "projects/Movement/SteeringBehaviors/Flocking/App_Flocking.h"
typedef App_Flocking CurrentApp;
#endif

#ifdef ActiveApp_GraphTheory
#include "projects/GraphTheory/App_GraphTheory.h"
typedef App_GraphTheory CurrentApp;
#endif

#ifdef ActiveApp_PathfindingAStar
#include "projects/Movement/Pathfinding/PathfindingAStar/App_PathfindingAStar.h"
typedef App_PathfindingAStar CurrentApp;
#endif

class App_Selector {
public: 
	static IApp* CreateApp() {
		IApp* myApp = new CurrentApp();
		return myApp;
	}
};