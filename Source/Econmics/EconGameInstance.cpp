// Fill out your copyright notice in the Description page of Project Settings.

#include "Econmics.h"
#include "EconGameInstance.h"

UEconGameInstance::UEconGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	
	/* This part helps to spawn blueprint-based cube. Mind the text link - don't rename the blueprint!
	We can't move this into Init() call since FObjectFinder is available only in constructor */
	static ConstructorHelpers::FObjectFinder<UBlueprint> BlockBaseBlueprint(TEXT("Blueprint'/Game/Economic/block_base_BP.block_base_BP'"));

	if (BlockBaseBlueprint.Object != NULL)
	{
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

	this->sim_world.spawnTestWorld();
	this->sim_world.constructSimulationEnvironment();

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
	
	TArray<SimGameBlock> chunk_blocks_array = this->sim_world.getActiveChunk();

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
	ABaseBlock *an_actor = this->spawned_actors_map[game_event.parent_gid];
	an_actor->python_simulation_new_event(game_event);
	return true;
}


bool UEconGameInstance::RunTestSimulationOnce(int32 time_units) {

	static FString repr("UEconGameInstance::RunTestSimulationOnce");

	bool isok = this->sim_world.runSimulationInterval(time_units);
	if (!isok) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - error with next simulation interval"), *repr);
		return false;
	}

	TArray<FGameEvent> EventsHappened = this->sim_world.recent_events;

	for (FGameEvent game_ev : EventsHappened) {
		this->scheduleGameEvent(game_ev);
	}

	return true;

	// This makes UE4 crash in 10 seconds. At least something.
	//TArray<uint32> PrimeNumbers;
	//FPrimeNumberWorker *NewWorker = FPrimeNumberWorker::JoyInit(PrimeNumbers, 5000);	

}

bool UEconGameInstance::StartTestThread() {
	UE_LOG(LogTemp, Warning, TEXT("Starting new thread"));

	/* This thing shall be managed by UE as a separate thread */
	FPyThreadWorker *myRunnable = NULL;
	myRunnable = new FPyThreadWorker();

	/* This is how we launch this runnable */
	FRunnableThread *the_thread;

	//windows default = 8mb for thread, could specify more
	the_thread = FRunnableThread::Create(myRunnable, TEXT("FPyThreadWorker"), 0, TPri_BelowNormal);

	/* This is how we clean up */

	//delete this->the_thread;
	//this->the_thread = NULL;


	return true;

}
