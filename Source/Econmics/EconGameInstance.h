
#pragma once

#include "CoreMinimal.h"
#include "BaseBlock.h"
#include "SimWorldInterface.h"
#include "PyThreadWorker.h"
#include "Engine/GameInstance.h"
#include "EconGameInstance.generated.h"


UCLASS()
class ECONMICS_API UEconGameInstance : public UGameInstance {
	GENERATED_BODY()

public:

	UEconGameInstance(const FObjectInitializer& ObjectInitializer);

	/* Start python on Init */
	void Init() override;

	/* Stop python on Shutdown */
	void Shutdown() override;

	/* Hold a reference to a child class of ABaseBlock that's made
	with blueprints. */
	TSubclassOf<class AStaticMeshActor> BlockBaseBlueprintClass;

	/*Generate a default level with python
	Return true when it's ok, false if it's not ok. */
	UFUNCTION(BlueprintCallable, Category="Simulation glue")
	bool GenerateDefaultLevel();

	/* Spawn the actors in an active chunck in the world. Call this only
	after generating the level. */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool SpawnActorsInActiveChunk();

	/* A link to SimWorldInterface instance that communicates with python code */
	SimWorldInterface sim_world;

	/* Testing the event system */
	TMap<long, ABaseBlock*> spawned_actors_map;

	/* Schedule animation event from simulation */
	bool scheduleGameEvent(FGameEvent game_event);

	/* Trying to run simulation for a fixed simulation time in a separate thread,
	python_simulation_new_event invoked for each event - these are all TODOs */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool RunTestSimulationOnce(int32 time_units);


	/* Multithread testing... */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool StartTestThread();

	/* Multithread testing... */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool DispatchTestSimulationEvents();

	/* Multithread testing... */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool StopTestThread();

	/* Multithread testing... */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool SetupRegularPullSimEventTimer();

	FTimerHandle simulation_checkup_timer_handle;

	void TestTimerCall();


	bool is_simulation_thread_running;
	FRunnableThread *simulation_thread;
	FPyThreadWorker *simulation_runnable;

};

