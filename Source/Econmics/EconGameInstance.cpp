// Fill out your copyright notice in the Description page of Project Settings.

#include "Econmics.h"
#include "EconGameInstance.h"

// Get references to some blueprints
UEconGameInstance::UEconGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	
	/* This part helps to spawn blueprint-based cube. Mind the text link - don't rename the blueprint!
	We can't move this into Init() call since FObjectFinder is available only in constructor */
	static ConstructorHelpers::FObjectFinder<UBlueprint> BlockBaseBlueprint(TEXT("Blueprint'/Game/Economic/block_base_BP.block_base_BP'"));

	if (BlockBaseBlueprint.Object != NULL) {
		this->BlockBaseBlueprintClass = (UClass*)BlockBaseBlueprint.Object->GeneratedClass;
	}
	else {
		this->BlockBaseBlueprintClass = NULL;
		UE_LOG(LogTemp, Warning, TEXT("[UEconGameInstance] - failed to get the block blueprint class reference"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UEconGameInstance] game instance constucted"));

}

// Start a background thread
void UEconGameInstance::Init() {

	UGameInstance::Init();  // A super call
	UE_LOG(LogTemp, Log, TEXT("[UEconGameInstance] Init"));

	// Start Python
	this->StartBackgroundPythonThread();

	// Generate a chunk (only one at the moment)
	this->GenerateDefaultLevel();

	this->SetupRegularPullSimEventTimer();  // timescale is set in StartBackgroundPythonThread() call

}

// Subcall of Init()
bool UEconGameInstance::StartBackgroundPythonThread() {

	if (this->is_simulation_thread_started) {
		UE_LOG(LogTemp, Warning, TEXT("Attempt to start already running thread"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Starting simulation thread"));

	/* This thing shall be managed by UE as a separate thread */
	FPyThreadWorker *MyRunnable = NULL;
	MyRunnable = new FPyThreadWorker();
	this->simulation_runnable = MyRunnable;  // hold the reference to collect it later
											 // windows default = 8mb for thread, could specify more
	FString ThreadName = TEXT("MyThreadWithSimulation");
	this->simulation_thread = FRunnableThread::Create(MyRunnable, *ThreadName, 0, TPri_BelowNormal);

	this->SetSimulationSpeedTimeScale(1.0f);  // a default value

	this->is_simulation_thread_started = true;
	return true;

}

// BP callable to set simulation timescale
bool UEconGameInstance::SetSimulationSpeedTimeScale(float timescale) {
	this->simulation_runnable->SetSimulationSpeedTimeScale(timescale);
	return true;
}

// Stop background thread before shutting down
void UEconGameInstance::Shutdown() {

	UE_LOG(LogTemp, Log, TEXT("[UEconGameInstance] Shutting down"));

	// Pause the simulation (not obligatory but would prevent python from
	// exessive calls).
	this->SetPySimulationSpeed(0);

	// Destroy the calling timer (not obligatory)
	this->GetWorld()->GetTimerManager().ClearTimer(this->simulation_checkup_timer_handle);

	// Stop python thread
	this->StopBackgroundPythonThread();

	UGameInstance::Shutdown();  // A super call

}

// subcall of Shutdown(), not a pause, this is termination
bool UEconGameInstance::StopBackgroundPythonThread() {
	// Clean up the thread
	if (!this->is_simulation_thread_started) {
		UE_LOG(LogTemp, Warning, TEXT("[StopTestThread] Attempt to stop a non-running thread"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[StopTestThread] Stopping simulation thread"));

	// WaitForCompletion() would invoke Stop() in some manner as well, but only after
	// Run() finishes execution. This way we ensure that the flag to stop Run() is
	// set up.
	this->simulation_runnable->AskToStop();
	this->simulation_thread->WaitForCompletion();

	if (this->simulation_thread) {
		delete this->simulation_thread;
		this->simulation_thread = nullptr;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[StopTestThread] simulation thread already deleted"));
	}

	// Deleting runnable before deleting the thread invokes an excpetion. But we still have
	// to do it to free the memory.
	if (this->simulation_runnable) {
		delete this->simulation_runnable;
		this->simulation_runnable = nullptr;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[StopTestThread] simulation runnable already deleted"));
	}

	UE_LOG(LogTemp, Log, TEXT("[StopTestThread] Stopped simulation thread"));

	this->is_simulation_thread_started = false;
	return true;

}


/* ~~~~~~~~~~ Commands to python thread ~~~~~~~~~~ */

// A comand to set an active chunk (=generate the level)
bool UEconGameInstance::GenerateDefaultLevel() {

	static FString repr("UEconGameInstance::GenerateDefaultLevel");

	UE_LOG(LogTemp, Log, TEXT("[%s] Generating level with Python"), *repr);

	this->simulation_runnable->SetActiveChunk(1);

	return true;

}

// A comand to spawn the world after the level is loaded
bool UEconGameInstance::SpawnActorsInActiveChunk() {

	static FString repr("UEconGameInstance::SpawnActorsInActiveChunk");

	UWorld* const World = GetWorld();
	if (World == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed to get the world reference"), *repr);
		return false;
	}

	if (this->BlockBaseBlueprintClass == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed to get the spawning class"), *repr);
		return false;
	}
	
	TArray<SimGameBlock> chunk_blocks_array = this->simulation_runnable->GetActiveChunk();

	// We have a reference to the base blueprint class, so we spawn a "child" class of ABaseBlock.

	for (SimGameBlock& GB_i : chunk_blocks_array) {

		FVector position(100 * GB_i.relX, 100 * GB_i.relY, 100 * GB_i.relZ);
		//FRotator rotator(0, 0, 0);  // We'll have to rotate it some day
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.bDeferConstruction = 1;  // We want to run it after we set the block type - so that, after the spawn.

		ABaseBlock *BPActor = World->SpawnActor<ABaseBlock>(this->BlockBaseBlueprintClass, position, FRotator::ZeroRotator, SpawnInfo);
		//UE_LOG(LogTemp, Warning, TEXT("[%s] - cube type is %d"), *repr, GB_i.cube_type);
		BPActor->cube_type = GB_i.cube_type;
		BPActor->gid = GB_i.gid;
		// BPActor->RerunConstructionScripts();  // Do it later
		this->spawned_actors_map.Add(GB_i.gid, BPActor);
	}

	/* Setup materials after the spawn */
	for (auto& Elem : this->spawned_actors_map) {
		Elem.Value->RerunConstructionScripts();
	}

	return true;
}



/* Simulation thread logic */

// Create a timer (even before the level is created)
bool UEconGameInstance::SetupRegularPullSimEventTimer() {

	this->is_paused = true;  // this ensures that timer won't do an actual call on setup
	this->GetWorld()->GetTimerManager().SetTimer(this->simulation_checkup_timer_handle,
		this, &UEconGameInstance::PeriodicSimulationTimerCall,
		this->simulation_checkup_timer_period, true);

	this->SetPySimulationSpeed(0);  // so that we start with a paused timer

	return true;

}

// BP callable
bool UEconGameInstance::SetPySimulationSpeed(int32 speed) {

	static FString repr("UEconGameInstance::SetPySimulationSpeed");

	auto tm = &this->GetWorld()->GetTimerManager();  // & is essential
	auto timer = this->simulation_checkup_timer_handle;

	this->ChangePySimSpeed(speed);  // modify sim engine speed first

	if (speed == 0) {
		// pause the timer, this makes the background thread stop,
		// it would be waiting for FEvent to trigger.
		this->is_paused = true;
		tm->PauseTimer(timer);
		UE_LOG(LogTemp, Log, TEXT("[%s] - timer is paused"), *repr);
		return true;
	}
	else if (tm->IsTimerPaused(timer)) {
		this->is_paused = false;
		tm->UnPauseTimer(timer);
		UE_LOG(LogTemp, Log, TEXT("[%s] - timer is unpaused"), *repr);
		return true;
	}

	return false;

}

// Change the simulation tick window itself
void UEconGameInstance::ChangePySimSpeed(int32 speed) {
	this->simulation_runnable->SetSimulationSpeed(speed);
}

// A regular timer call
void UEconGameInstance::PeriodicSimulationTimerCall() {
	// Timer stops anyway, this "return" statement just ensures
	// that simulation stops early on pause button and that timer
	// won't do any harm on startup.
	if (this->is_paused) {
		UE_LOG(LogTemp, Warning, TEXT("A timer call on pause"));
		return;
	}

	this->GetAndScheduleSimulationEventsFromBackgroundThread();

}

// Communicate with python
bool UEconGameInstance::GetAndScheduleSimulationEventsFromBackgroundThread() {
	if (!this->is_simulation_thread_started) {
		UE_LOG(LogTemp, Warning, TEXT("Attempt to read events when no simulation active"));
		return false;
	}

	// After these two calls we release the simulation, so that it does not wait
	// for hashmaps to route the data to actors.
	TArray<FPySimGameEvent> recent_events = this->simulation_runnable->GetRecentEvents();
	TArray<TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe>> data_updates = this->simulation_runnable->GetDataUpdates();
	
	// FIXME: we don't need two repeatative lookups of actors. But we can't fix it here.

	for (auto ev_i : recent_events) {
		/* These are FPySimGameEvent */
		this->ScheduleBPAnimationOfSimulationEvent(ev_i);
	}

	for (auto data_i : data_updates) {
		this->ScheduleBPAnimationDataUpdate(data_i);
	}

	return true;

}

// Animate an event - not very useful though
bool UEconGameInstance::ScheduleBPAnimationOfSimulationEvent(FPySimGameEvent game_event) {
	if (this->spawned_actors_map.Contains(game_event.parent_gid)) {
		ABaseBlock *an_actor = this->spawned_actors_map[game_event.parent_gid];
		an_actor->ApplyPySimGameEvent(game_event);
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Can't find an actor with gid=%d"), game_event.parent_gid);
		return false;
	}
}


bool UEconGameInstance::ScheduleBPAnimationDataUpdate(TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe> behaviour_data_update) {
	if (this->spawned_actors_map.Contains(behaviour_data_update->parent_gid)) {
		ABaseBlock *an_actor = this->spawned_actors_map[behaviour_data_update->parent_gid];
		an_actor->ApplyPyDataUpdate(behaviour_data_update);
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Can't find an actor with gid=%d"), behaviour_data_update->parent_gid);
		return false;
	}
}
