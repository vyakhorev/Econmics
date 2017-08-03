// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <string>
#include "SharedPy.h"
#include "EvStruct.h" // for FGameEvent structure, this is how we animate simulation events

using namespace std;

/* Helps to transfer (and store) info from python to C++ usages */
struct SimGameBlock {
	long relX;
	long relY;
	long relZ;
	long cube_type;
	long gid;

	/* Holds a reference to the parent spawned actor,
	it's actor responsibility to connect and disconnect
	from a SimGameBlock */
	//AStaticMeshActor parent_actor;

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
	bool spawnTestWorld();
	TArray<SimGameBlock> getActiveChunk();

	/* Test simulation. Constructs simulation environment */
	bool constructSimulationEnvironment();

	/* Test simulation */
	bool runSimulationInterval(long interval_tu);
	TArray<FGameEvent> recent_events;

	/* A way to pass and buffer the world in the C++ memory,
	iterator over Python data would be better... */
	TMap<long, SimGameBlock> game_block_map;


private:

	/* Adds the block to the internal map. This arhitecture should be tuned somehow.. */
	bool addNewBlock(long relX, long relY, long relZ, long cube_type, long gid);

	/* Adds a simulation event into internal TArray */
	bool registerRecentEvent(PyObject *sim_event);

	/* Starts python only if not is_python_working */
	bool safeStartPython();
	/* Stops python only if is_python_working */
	bool safeStopPython();
	bool is_python_working;

	/* imports and links ue4exec.py */
	bool importPyInterfaceModule();
	
	/* A link to the cSimWorld instance */
	PyObject *py_world_instance;
	/* A link to the ue4exec.py module. All the python calls should be
	available from that module. */
	PyObject *py_ue4exec_module;
	/* A link to cSimEnvironment with started world simulation */
	PyObject *py_simulation_environment;

};

