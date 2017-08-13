#include "Econmics.h"
#include "PyThreadWorker.h"

/* ~~~~~~~~~~ Basic mechanics ~~~~~~~~~~ */

FPyThreadWorker::FPyThreadWorker() {

	this->sim_cycle_semaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);

	/* A flag to stop the Run() loop. */
	this->flag_is_simulation_stopped = false;

}

FPyThreadWorker::~FPyThreadWorker() {

	//Cleanup the FEvent
	if (this->sim_cycle_semaphore) {
		FGenericPlatformProcess::ReturnSynchEventToPool(this->sim_cycle_semaphore);
		this->sim_cycle_semaphore = nullptr;
	}

}

bool FPyThreadWorker::Init() {

	this->recent_events.Empty();
	return true;

}

uint32 FPyThreadWorker::Run() {

	/* First simulation iteration */
	if (!this->flag_is_simulation_stopped) {
		this->DoPython();
	}

	while (!this->flag_is_simulation_stopped) {
		/* Straight forward implementation: do the tick and wait.
		A lot of optimisations available here. Semafore would
		be triggered when the game thread calls GetRecentEvents().
		We have to wait first so that AskToStop() would work. */
		this->sim_cycle_semaphore->Wait();
		this->DoPython();
	}

	return 0;
}

void FPyThreadWorker::Stop() {

	UE_LOG(LogTemp, Warning, TEXT("[FPyThreadWorker] Stop"));

}

void FPyThreadWorker::AskToStop() {
	/* This order is important in order to unlock Run() cycle */
	this->flag_is_simulation_stopped = true;
	this->sim_cycle_semaphore->Trigger();

	UE_LOG(LogTemp, Warning, TEXT("[FPyThreadWorker] AskToStop"));

}

/* ~~~~~~~~~~ Command interface ~~~~~~~~~~ */

/* Game thread call to get the simulated data */
TArray<FGameEvent> FPyThreadWorker::GetRecentEvents() {

	static FString repr("FPyThreadWorker::GetRecentEvents");

	TArray<FGameEvent> ans_array;
	ans_array.Empty();
	if (this->flag_is_simulation_stopped) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] attempt to get events with a stopped simulation"), *repr);
		return ans_array;
	}

	if (this->recent_events_mutex.TryLock()) {
		ans_array.Append(this->recent_events);
		this->recent_events.Empty();
		this->recent_events_mutex.Unlock();
		/* Release simulation to go further */
		this->sim_cycle_semaphore->Trigger();

		return ans_array;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[%s] failed to lock recent_events array for read-write access"), *repr);
		return ans_array;
	}

}

/* ~~~~~~~~~~ Internal logic ~~~~~~~~~~ */

bool FPyThreadWorker::DoPython() {
	this->ThreadSafeRunSimEventsTick(0.1f);
}




bool FPyThreadWorker::ThreadSafeRunSimEventsTick(float const dt) {

	static FString repr("FPyThreadWorker::RunSimEventsTick");

	/* Lock the possibility to get recent events */
	if (!this->recent_events_mutex.TryLock()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] failed to lock recent_events array, skipping simulation tick"), *repr);
		return false;
	}

	/* Fill internal array with recent events (python simulation should
	be callled here). The aray is emptied when we get the events. */
	this->CalculateRecentEvents(dt);

	this->recent_events_mutex.Unlock();

	return true;
}

void FPyThreadWorker::CalculateRecentEvents(float const dt) {
	 
	/* Fake setup */
	static int32 block_count = 1000;
	static int32 min_events = 0;
	static int32 max_events = 100;
	
	int32 event_num = FMath::RandRange(min_events, max_events);

	for (int32 i = 0; i < event_num; i++) {
		int32 gid = FMath::RandRange(1, block_count);

		FGameEvent new_event = FGameEvent();
		new_event.event_description = "hello from another thread";
		new_event.parent_gid = gid;

		/* Should be already thread sage at the moment */
		this->recent_events.Add(new_event);
	}

	// Emulate some delay with simulation calculations
	//FPlatformProcess::Sleep(0.1);

}