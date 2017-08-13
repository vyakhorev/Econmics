// Fill out your copyright notice in the Description page of Project Settings.

#include "Econmics.h"
#include "EconGameInstance.h"

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

void UEconGameInstance::Init() {

	UGameInstance::Init();  // A super call

	UE_LOG(LogTemp, Log, TEXT("[UEconGameInstance] Init"));

	/* Start python interface (it would start python as well,
	however, python lifespan in memory is sim_world's
	responsibility. */

	this->sim_world = SimWorldInterface();
	this->sim_world.Init();
	
}

void UEconGameInstance::Shutdown() {

	UE_LOG(LogTemp, Log, TEXT("[UEconGameInstance] Shutting down"));

	/* Stop python interface. This means shutting down python as well.
	Without this call python memory somehow stays between calls,
	very dramatic bugs I had those times.. */
	this->sim_world.Shutdown();

	UGameInstance::Shutdown();  // A super call

}

bool UEconGameInstance::GenerateDefaultLevel() {

	static FString repr("UEconGameInstance::GenerateDefaultLevel");

	UE_LOG(LogTemp, Log, TEXT("[%s] Generating level with Python"), *repr);

	this->sim_world.SpawnTestWorld();
	this->sim_world.ConstructSimulationEnvironment();

	return true;

}

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
	
	TArray<SimGameBlock> chunk_blocks_array = this->sim_world.GetActiveChunk();

	// We have a reference to the base blueprint class, so we spawn a "child" class of ABaseBlock.

	for (SimGameBlock& GB_i : chunk_blocks_array) {

		FVector position(100 * GB_i.relX, 100 * GB_i.relY, 100 * GB_i.relZ);
		//FRotator rotator(0, 0, 0);  // We'll have to rotate it some day
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.bDeferConstruction = 1;  // We want to run it after we set the block type - so that, after the spawn.

		ABaseBlock *BPActor = World->SpawnActor<ABaseBlock>(this->BlockBaseBlueprintClass, position, FRotator::ZeroRotator, SpawnInfo);
		BPActor->cube_type = GB_i.cube_type;
		BPActor->gid = GB_i.gid;
		BPActor->RerunConstructionScripts();  // Run the thing
		this->spawned_actors_map.Add(GB_i.gid, BPActor);
	}

	return true;
}

bool UEconGameInstance::scheduleGameEvent(FGameEvent game_event) {
	if (this->spawned_actors_map.Contains(game_event.parent_gid)) {
		ABaseBlock *an_actor = this->spawned_actors_map[game_event.parent_gid];
		an_actor->python_simulation_new_event(game_event);
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Can't find an actor with gid=%d"), game_event.parent_gid);
		return false;
	}
	
}


bool UEconGameInstance::RunTestSimulationOnce(int32 time_units) {

	static FString repr("UEconGameInstance::RunTestSimulationOnce");

	bool isok = this->sim_world.RunSimulationInterval(time_units);
	if (!isok) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - error with next simulation interval"), *repr);
		return false;
	}

	TArray<FGameEvent> EventsHappened = this->sim_world.recent_events;

	for (FGameEvent game_ev : EventsHappened) {
		this->scheduleGameEvent(game_ev);
	}

	return true;

}



/* Simulation thread logic */

bool UEconGameInstance::StartTestThread() {

	if (this->is_simulation_thread_running) {
		UE_LOG(LogTemp, Warning, TEXT("Attempt to start already running thread"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Starting simulation thread"));

	/* This thing shall be managed by UE as a separate thread */
	FPyThreadWorker *MyRunnable = NULL;
	MyRunnable = new FPyThreadWorker();
	this->simulation_runnable = MyRunnable;  // hold the reference to collect it later

	// windows default = 8mb for thread, could specify more
	FString ThreadName = TEXT("MyThreadWithSimulation");
	this->simulation_thread = FRunnableThread::Create(MyRunnable, *ThreadName, 0, TPri_BelowNormal);

	this->is_simulation_thread_running = true;
	return true;

}

bool UEconGameInstance::DispatchTestSimulationEvents() {
	if (!this->is_simulation_thread_running) {
		UE_LOG(LogTemp, Warning, TEXT("Attempt to read events when no simulation active"));
		return false;
	}

	// This can lock game thread, so we need to be smart about it.
	// May be reading a single event is not such a bad idea..
	TArray<FGameEvent> recent_events = this->simulation_runnable->GetRecentEvents();

	UE_LOG(LogTemp, Warning, TEXT("DispatchTestSimulationEvents!!!"));

	for (auto ev_i : recent_events) {
		/* These are FGameEvent */
		this->scheduleGameEvent(ev_i);
	}

	return true;

}

void UEconGameInstance::TestTimerCall() {
	UE_LOG(LogTemp, Warning, TEXT("Timer calls"));
	this->DispatchTestSimulationEvents();
}

bool UEconGameInstance::SetupRegularPullSimEventTimer() {

	//this->GetWorld()->GetTimerManager().SetTimer(this->simulation_checkup_timer, this, )
	//FTimerDelegate MyDel;
	//MyDel.BindUFunction(this, "DispatchTestSimulationEvents");
	//FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &UEconGameInstance::DispatchTestSimulationEvents);

	this->GetWorld()->GetTimerManager().SetTimer(this->simulation_checkup_timer_handle,
		this, &UEconGameInstance::TestTimerCall, 0.1f, true);

	return true;

}

bool UEconGameInstance::StopTestThread() {
	// Clean up the thread
	if (!this->is_simulation_thread_running) {
		UE_LOG(LogTemp, Warning, TEXT("[StopTestThread] Attempt to stop a non-running thread"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("[StopTestThread] Stopping simulation thread"));

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

	UE_LOG(LogTemp, Warning, TEXT("[StopTestThread] Stopped simulation thread"));

	this->is_simulation_thread_running = false;
	return true;

}
