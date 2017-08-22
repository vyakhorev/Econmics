#include "Econmics.h"
#include "PyThreadWorker.h"

/* ~~~~~~~~~~ Basic mechanics ~~~~~~~~~~ */

FPyThreadWorker::FPyThreadWorker() {

	this->sim_cycle_semaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);

	/* A flag to stop the Run() loop. */
	this->flag_is_simulation_stopped = false;

	/* Default timescale of simulation */
	this->sim_time_timescale = 1.0f;

}

FPyThreadWorker::~FPyThreadWorker() {

	//Cleanup the FEvent
	if (this->sim_cycle_semaphore) {
		FGenericPlatformProcess::ReturnSynchEventToPool(this->sim_cycle_semaphore);
		this->sim_cycle_semaphore = nullptr;
	}

}

bool FPyThreadWorker::Init() {

	this->data_updates.Empty();

	/* Start python interface (it would start python as well,
	however, python lifespan in memory is sim_world's
	responsibility. */
	this->sim_world = SimWorldInterface();
	bool python_started = this->sim_world.Init();  // starts python (so that it starts in a separate thread

	return python_started;

}

uint32 FPyThreadWorker::Run() {

	// !!! NOTE. this is not the best possible design. We don't need events actually.
	// But we should be able to advance simulation futher without receiving anything
	// (optimisations for very high speed).

	while (!this->flag_is_simulation_stopped) {
		/* Straight forward implementation: do the tick and wait.
		A lot of optimisations available here. Semafore would
		be triggered when the game thread calls GetRecentEvents().
		We have to wait first so that AskToStop() would work. */
		this->sim_cycle_semaphore->Wait();  // GetRecentEvents()
		this->ApplyPythonSimulationTick();
	}

	UE_LOG(LogTemp, Warning, TEXT("[FPyThreadWorker] Stopped running"));

	return 0;
}

void FPyThreadWorker::Stop() {
	/* It's important to have Stop() separted from AskToStop() since 
	at this point the Run() is finished and we can safely stop python
	from the same thread that started it. Otherwise we may have python
	commands being called after we finilized python itself which 
	will result in a crash. */

	UE_LOG(LogTemp, Warning, TEXT("[FPyThreadWorker] Stop"));

	/* Stop python interface. This means shutting down python as well.
	Without this call python memory somehow stays between calls,
	very dramatic bugs I had those times.. */
	this->sim_world.Shutdown();
	
}

/* Game thread call to ensure that Run() would soon stop and
the game thread would succeed with waiting for termination */
void FPyThreadWorker::AskToStop() {
	/* This order is important in order to unlock Run() cycle */
	this->flag_is_simulation_stopped = true;
	this->sim_cycle_semaphore->Trigger();

	UE_LOG(LogTemp, Warning, TEXT("[FPyThreadWorker] AskToStop"));

}


/* ~~~~~~~~~~ Command interface ~~~~~~~~~~ */

/* Game thread call to indicate the fact that we've changed
the planet (=chunk) 
!!! It's very important to call this only when the timer is
suspended (otherwise we'll have 2 threads trying to call
python functions. */
bool FPyThreadWorker::SetActiveChunk(uint32 chunk_gid) {
	this->sim_world.SpawnTestWorld();
	return true;
}

/* Game thread call to get active chunk */
TArray<SimGameBlock> FPyThreadWorker::GetActiveChunk() {
	return this->sim_world.GetActiveChunk();
}

TArray<TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe>> FPyThreadWorker::GetDataUpdates() {
	
	static FString repr("FPyThreadWorker::GetDataUpdates");

	TArray<TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe>> ans_array;
	ans_array.Empty();
	if (this->flag_is_simulation_stopped) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] attempt to get data updates with a stopped simulation"), *repr);
		return ans_array;
	}

	if (this->data_updates_mutex.TryLock()) {
		/* Smart thread safe UE4 pointers are transfered from
		background to game thread here. */
		ans_array.Append(this->data_updates);
		this->data_updates.Empty();
		this->data_updates_mutex.Unlock();
		/* Release simulation to go futher */
		this->sim_cycle_semaphore->Trigger();

		return ans_array;

	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[%s] failed to lock data_updates array for read-write access"), *repr);
		return ans_array;
	}

}

/* Game thread call to change simulation speed or even pause the simulation */
void FPyThreadWorker::SetSimulationSpeed(int32 new_speed) {
	this->sim_time_per_tick = new_speed * this->sim_time_timescale;
}

/* Game thread call to change timescale of SetSimulationSpeed calls */
void FPyThreadWorker::SetSimulationSpeedTimeScale(float timescale) {
	this->sim_time_timescale = timescale;
}


/* ~~~~~~~~~~ Internal logic ~~~~~~~~~~ */

void FPyThreadWorker::ApplyPythonSimulationTick() {
	// Do the simulation itself. In essesnce, chages states in Python variables.
	this->ThreadSafeRunSimEventsTick(this->sim_time_per_tick);
	// Get data updates that were prepared by python (currently they are requested
	// for each event, but we can request them when needed).
	this->ThreadSafeGatherAnimationData();
}

bool FPyThreadWorker::ThreadSafeRunSimEventsTick(float dt) {

	static FString repr("FPyThreadWorker::RunSimEventsTick");

	/* Do the calculations, silently. */
	this->CalculateRecentEvents(dt);

	return true;
}

void FPyThreadWorker::CalculateRecentEvents(float dt) {

	this->sim_world.RunSimulationInterval(dt);

}

bool FPyThreadWorker::ThreadSafeGatherAnimationData() {
	static FString repr("FPyThreadWorker::ThreadSafeGatherAnimationData");

	/* Lock the possibility to get recent events */
	if (!this->data_updates_mutex.TryLock()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] failed to lock data_updates array, skipping data update"), *repr);
		return false;
	}

	/* Fill internal array with data updates. It's emptied when the client calls GetDataUpdates() */
	this->GatherAnimationData();

	this->data_updates_mutex.Unlock();

	return true;

}

void FPyThreadWorker::GatherAnimationData() {

	this->sim_world.GatherAnimationUpdates();
	this->data_updates.Append(this->sim_world.behaviour_updates);

}