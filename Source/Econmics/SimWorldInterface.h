// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <string>
#include "SharedPy.h"
#include "EvStruct.h"  // for FPySimGameEvent structure to be informed about events
// Simulation states
//#include "BehComponents/BehBlooming.h"

using namespace std;

/* Helps to transfer (and store) info from python to C++ usages */
struct SimGameBlock {
	long relX;
	long relY;
	long relZ;
	long cube_type;
	long gid;

	SimGameBlock() {
		relX = 0;
		relY = 0;
		relZ = 0;
		gid = 0;
		cube_type = 0;
	};

};


/* Provides interface to python simulation engine and interaction with python world
Does the python loading. */
class SimWorldInterface {

public:
	SimWorldInterface();
	~SimWorldInterface();

	/* Call this when game instance started to work, currently this
	starts Python in the same thread */
	bool Init();
	/* Call this when game instance shuts down, currently this
	stops Python */
	bool Shutdown();

	/* Test spawn. Populates GameBlockMap and returns true in case of success. */
	bool SpawnTestWorld();

	/* Returns the spawned world */
	TArray<SimGameBlock> GetActiveChunk();

	/* Simulation call */
	bool RunSimulationInterval(float interval_tu);
	TArray<FPySimGameEvent> recent_events;

	/* Collect updates about cube states */
	bool GatherAnimationUpdates();
	//TArray<FPyBehBlooming> beh_100_updates;


	/* A way to pass and buffer the world in the C++ memory,
	iterator over Python data would be better... */
	TMap<long, SimGameBlock> game_block_map;


private:

	/* Adds the block to the internal map. This arhitecture should be tuned somehow.. */
	bool AddNewBlock(long relX, long relY, long relZ, long cube_type, long gid);

	/* Adds a simulation event into internal TArray */
	bool RegisterRecentEvent(PyObject *sim_event);

	/* Starts python only if not is_python_working */
	bool SafeStartPython();
	/* Stops python only if is_python_working */
	bool SafeStopPython();
	bool is_python_working;

	/* imports and links ue4exec.py */
	bool ImportPyInterfaceModule();

	/* A reference to cSimulationController instance */
	PyObject *py_simulation_controller;

	/* A link to the ue4exec.py module. All the python calls should be
	available from that module. */
	PyObject *py_ue4exec_module;

};

