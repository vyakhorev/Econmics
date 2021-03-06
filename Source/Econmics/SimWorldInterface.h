// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <string>
#include "SharedPy.h"
// Simulation states
#include "BasicBehaviour.h"
#include "BehBlooming.h"
#include "BehTemperature.h"
#include "BehVapors.h"
#include "BehChemistry.h"
#include "BehBiomass.h"


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

	/* Progress the simulation futher */
	bool RunSimulationInterval(float interval_tu);

	/* Collect updates about cube states inside this object */
	bool GatherAnimationUpdates();

	/* Store animation updates in a buffer. With a very scary typename. 
	This thing shall keep counting refences to behaviours even when there
	are in other thread (otherwise I received garbage data). */
	TArray<TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe>> behaviour_updates;
	//TArray<FPyBasicBehaviour*> behaviour_updates;

	/* A way to pass and buffer the world in C++ memory from Python. */
	TMap<long, SimGameBlock> game_block_map;


private:

	/* Adds the block to the internal map. This arhitecture should be tuned somehow.. */
	bool AddNewBlock(long relX, long relY, long relZ, long cube_type, long gid);

	/* Adds animation update to internal buffer
	IMPORTANT: here we link roles with behaviour objects */
	bool RegisterAnimationUpdate(PyObject *update_tuple);

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

