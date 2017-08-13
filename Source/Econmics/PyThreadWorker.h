#pragma once

#include "CoreMinimal.h"
#include "EvStruct.h"  // this is how game thread animates events

class FPyThreadWorker : public FRunnable {

public:

	//Constructor / Destructor
	FPyThreadWorker();
	virtual ~FPyThreadWorker();

	/* FRunnable interface. These calls would be invoked by
	FRunnableThread. Everything else is custom. */
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	/* Ask Run() cycle to stop before finnaly stopping the thread. */
	void AskToStop();

	/* A method to get recent simulation events, called from the game thread,
	the events are copied from test_events (and simulation halts while they're
	transfered). Run() cycle halts after a simulation tick, this call transfers
	the data and releases the Run() cycle. So The game can just call this mthod
	periodically (not too often).*/
	TArray<FGameEvent> GetRecentEvents();


private:

	/* Aggregated call to Python calls, called with a timer*/
	bool DoPython();


	/* Simulation call, populates recent_events. This is repeatedly called
	within Run() cycle. */
	bool ThreadSafeRunSimEventsTick(float const dt);

	/* The actual thread unsafe population of recent_events array */
	void CalculateRecentEvents(float const dt);

	/* A thing to indicate a blocking in a thread-safe way */
	FCriticalSection recent_events_mutex;

	/* This FEvent helps to snooze and wake up simulation thread.
	Wait() is called in Run(), Trigger() is called in GetRecentEvents(). */
	FEvent *sim_cycle_semaphore;

	/* A flag to indicate the Run() cycle to stop. Set up from game thread,
	accessed in simulation thread - that's why it's thread safe. Don't know
	how it works though. */
	FThreadSafeBool flag_is_simulation_stopped;

	/* Hold simulated events here */
	TArray<FGameEvent> recent_events;


};