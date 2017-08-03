#pragma once

#include "CoreMinimal.h"

class FPyThreadWorker : public FRunnable {

public:

	//Constructor / Destructor
	FPyThreadWorker();
	virtual ~FPyThreadWorker();

	// FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();


	/* Thread to run the worker FRunnable on */
	// FRunnableThread *the_thread;


	///* A pointer to the data */
	//TArray<long> *test_events;

	///* Some test calculations */
	//bool GenerateFakeEvents();

};

