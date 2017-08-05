#pragma once

#include "CoreMinimal.h"

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
	TArray<uint32> GetRecentEvents();


private:

	/* Simulation call, populates test_events */
	bool RunSimEventsTick(float dt);

	/* A thing to indicate a blocking in a thread-safe way */
	FCriticalSection m_mutex;

	/* A thing to suspend and bring back to life this thread */
	FEvent *m_semaphore;

	/* A flag to indicate the Run() cycle to stop. Set up from game thread,
	accessed in simulation thread - that's why it's thread safe. Don't know
	how it works though. */
	FThreadSafeBool m_stop_simulation;

	//FThreadSafeBool on_pause;

	/* Hold simulated events here */
	TArray<uint32> recent_events;


};