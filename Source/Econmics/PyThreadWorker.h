#pragma once


#include "CoreMinimal.h"
#include "SimWorldInterface.h"  // Python calls interface

class FPyThreadWorker : public FRunnable {

public:

	/* ~~~~~~~~~~ Basic mechanics ~~~~~~~~~~ */

	//Constructor / Destructor
	FPyThreadWorker();
	virtual ~FPyThreadWorker();

	/* FRunnable interface. These calls would be invoked by
	FRunnableThread. Everything else is custom. */
	virtual bool Init();  // Start python here
	virtual uint32 Run();
	virtual void Stop();  // Stop python here

	/* Ask Run() cycle to stop before finnaly stopping the thread. */
	void AskToStop();  // Stop a loop + unlock everything here


	/* ~~~~~~~~~~ Command interface ~~~~~~~~~~ */

	/* Returns data updates. Releses simulation. */
	TArray<TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe>> GetDataUpdates();

	/* A command to change the active chunk (would load / generate it
	in Python memory) */
	bool SetActiveChunk(uint32 chunk_gid);

	/* A command to get active chunk. Uses recent_events_mutex. */
	TArray<SimGameBlock> GetActiveChunk();

	/* Sets new speed for simulation, this is called when we change
	speed of the game with buttons */
	void SetSimulationSpeed(int32 new_speed);

	/* Sets the scale factor from buttons (int32) to actual sim time (float)
	This should be called only once (or between changing regimes from planet
	to galaxy) */
	void SetSimulationSpeedTimeScale(float timescale);


private:

	/* A link to SimWorldInterface instance that communicates with python code */
	SimWorldInterface sim_world;

	/* ~~~~~~~~~~ Simulation handling  ~~~~~~~~~~  */

	/* Each time the game thread calls for GetRecentEvents(), we silently
	generate new ones with python. */

	/* Repeatitive call to python simulation: simulate new events and store
	them in a buffer recent_events, apply all the commands after it (block
	construction, settings, etc) */
	void ApplyPythonSimulationTick(); 

	/* This defines simulation speed (timer is usually frequent).  */
	float sim_time_per_tick;

	/* Defines how x1, x2 ... transforms into actual simulation time */
	float sim_time_timescale;

	/* Simulation call. Does python calculations without communicating.
	In order to get animation updates use ThreadSafeGatherAnimationData*/
	bool ThreadSafeRunSimEventsTick(float dt);

	/* The actual call to simulation. */
	void CalculateRecentEvents(float dt);

	/* This FEvent helps to snooze and wake up simulation thread.
	Wait() is called in Run(), Trigger() is called in GetRecentEvents(). */
	FEvent *sim_cycle_semaphore;

	/* A flag to indicate the Run() cycle to stop. Set up from game thread,
	accessed in simulation thread - that's why it's thread safe. Don't know
	how it works though. */
	FThreadSafeBool flag_is_simulation_stopped;

	/* Mutex over data_updates array */
	FCriticalSection data_updates_mutex;

	/* Holds all the data updates that were received from python. 
	sim_world empties it's array every tick. If the game thread
	is not synchronised with this thread, data could be lost.
	That's why I added additional buffer. We can get rid of it. */
	TArray<TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe>> data_updates;


	/* Data collection threadsafe wrapper */
	bool ThreadSafeGatherAnimationData();

	/* Data collection itself */
	void GatherAnimationData();


};