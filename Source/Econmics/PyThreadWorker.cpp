#include "Econmics.h"
#include "PyThreadWorker.h"

FPyThreadWorker::FPyThreadWorker() {
	// Not sure I want this in constructor..

	this->m_semaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);
	this->m_stop_simulation = false;

}

FPyThreadWorker::~FPyThreadWorker() {

	if (this->m_semaphore) {
		//Cleanup the FEvent
		FGenericPlatformProcess::ReturnSynchEventToPool(this->m_semaphore);
		this->m_semaphore = nullptr;
	}

}

bool FPyThreadWorker::Init() {
	this->recent_events.Empty();
	return true;
}

uint32 FPyThreadWorker::Run() {

	while (!this->m_stop_simulation) {
		// Straight forward implementation: do the tick and wait.
		// A lot of optimisations available here. Semafore would
		// be triggered when the game thread calls GetRecentEvents().
		this->RunSimEventsTick(0.1f);
		this->m_semaphore->Wait();
	}

	return 0;
}

void FPyThreadWorker::Stop() {

	UE_LOG(LogTemp, Warning, TEXT("[FPyThreadWorker] Stop"));

}

void FPyThreadWorker::AskToStop() {
	// Set the flag to stop simulation.

	this->m_stop_simulation = true;

	UE_LOG(LogTemp, Warning, TEXT("[FPyThreadWorker] AskToStop"));

}

/* Simulate with RunSimEventsTick (called in Run() ), get the events with GetRecentEvents */

TArray<uint32> FPyThreadWorker::GetRecentEvents() {

	UE_LOG(LogTemp, Warning, TEXT("[GetRecentEvents] lock the mutex"));

	TArray<uint32> ans_array;
	ans_array.Empty();
	if (this->m_stop_simulation) {
		UE_LOG(LogTemp, Warning, TEXT("[GetRecentEvents] no simulation running"));
		return ans_array;
	}

	if (this->m_mutex.TryLock()) {
		UE_LOG(LogTemp, Warning, TEXT("[GetRecentEvents] mutex locked, getting the data"));
		ans_array.Append(this->recent_events);
		this->recent_events.Empty();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[GetRecentEvents] lock failed, providing with empty list "));
		return ans_array;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GetRecentEvents] unlocking the mutex"));
	this->m_mutex.Unlock();
	UE_LOG(LogTemp, Warning, TEXT("[GetRecentEvents] mutex unlocked"));

	// Release simulation to go further
	this->m_semaphore->Trigger();

	return ans_array;

}

bool FPyThreadWorker::RunSimEventsTick(float dt) {

	UE_LOG(LogTemp, Warning, TEXT("[RunSimEventsTick] lock the mutex"));

	/* Lock the possibility to get recent events */
	if (!this->m_mutex.TryLock()) {
		UE_LOG(LogTemp, Warning, TEXT("[RunSimEventsTick] Skipping simulation tick"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("[RunSimEventsTick] mutex locked, simulating"));

	/* Fill internal array with recent events (python simulation should
	be callled here). The aray is emptied when we get the events. */
	this->recent_events.Add(10);
	this->recent_events.Add(20);
	this->recent_events.Add(30);
	this->recent_events.Add(40);
	this->recent_events.Add(50);

	// Emulate delay with simulation calculations
	UE_LOG(LogTemp, Warning, TEXT("[RunSimEventsTick] Doing cadabra"));
	FPlatformProcess::Sleep(0.2);
	UE_LOG(LogTemp, Warning, TEXT("[RunSimEventsTick] Finished cadabra"));

	UE_LOG(LogTemp, Warning, TEXT("[RunSimEventsTick] unlocking the mutex"));
	this->m_mutex.Unlock();
	UE_LOG(LogTemp, Warning, TEXT("[RunSimEventsTick] mutex unlocked"));

	return true;

}