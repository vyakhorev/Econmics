#include "Econmics.h"
#include "PyThreadWorker.h"

FPyThreadWorker::FPyThreadWorker() {
	// Not sure I want this in constructor..
	//this->the_thread = FRunnableThread::Create(this, TEXT("FPyThreadWorker"), 0, TPri_BelowNormal);  //windows default = 8mb for thread, could specify more
}

FPyThreadWorker::~FPyThreadWorker() {
	//delete this->the_thread;
	//this->the_thread = NULL;
}

//Init
bool FPyThreadWorker::Init() {
	//Init the Data 
	//this->test_events
	return true;
}

//Run
uint32 FPyThreadWorker::Run() {
	//Initial wait before starting
	FPlatformProcess::Sleep(5);
	return 0;
}

//stop
void FPyThreadWorker::Stop() {
	// again, nothing to do here yet
}