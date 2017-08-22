
#pragma once

#include "CoreMinimal.h"
#include "BaseBlock.h"
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

	/* ~~~~~~~~~~ Actors spawning ~~~~~~~~~~ */

	/* Hold a reference to a child class of ABaseBlock that's made
	with blueprints. */
	TSubclassOf<class AStaticMeshActor> BlockBaseBlueprintClass;

	/*Generate a default level with python
	Return true when it's ok, false if it's not ok. */
	bool GenerateDefaultLevel();

	/* Spawn the actors in an active chunck in the world. this is called
	in the level blueprint. */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool SpawnActorsInActiveChunk();

	/* This map is filled in SpawnActorsInActiveChunk() */
	TMap<long, ABaseBlock*> spawned_actors_map;

	/* ~~~~~~~~~~ Timer and simulation - commands ~~~~~~~~~~ */

	/* Set the simulation speed in a general way. When speed=0 simulation
	timer is paused. Other values change the time window in python tick */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool SetPySimulationSpeed(int32 speed);

	/* Sets the timescale of simulation, changes how simulation responds
	too SetPySimulationSpeed() calls */
	UFUNCTION(BlueprintCallable, Category = "Simulation glue")
	bool SetSimulationSpeedTimeScale(float timescale);



private:

	/* ~~~~~~~~~~ Timer and simulation ~~~~~~~~~~ */

	/* A flag that indicates that the background python thread
	is currently active (it may be not active only before destruction) */
	bool is_simulation_thread_started;
	/* Additional reference that controls the simulation_runnable */
	FRunnableThread *simulation_thread;
	/* Main reference to all Python calls */
	FPyThreadWorker *simulation_runnable;

	/* On Init() this class starts a python thread that
	handles all the simulation and internal world content */
	bool StartBackgroundPythonThread();

	/* On Shutdown() this class waits for python to finish */
	bool StopBackgroundPythonThread();

	/* Changes the speed of python simulation in a thread safe way */
	void ChangePySimSpeed(int32 speed);

	/* Timer speed. Should be higher than simulation calculation time
	(otherwise some of the calls won't return a result) */
	float simulation_checkup_timer_period = 0.1f;

	/* A handle to the timer that collects simulation events */
	FTimerHandle simulation_checkup_timer_handle;

	/* A flag to ensure that timer won't do any unnesesary calls */
	bool is_paused;

	/* A timer that updates simulations, paused after this call */
	bool SetupRegularPullSimEventTimer();

	/* This is called every timer tick */
	void PeriodicSimulationTimerCall();

	/* Communicates with background python thread and gets the events
	(this makes the background thread simulate more events). */
	bool GetAndScheduleSimulationEventsFromBackgroundThread();

	/* Schedule animation event from simulation (more like 'trigger' at the moment) */
	bool ScheduleBPAnimationOfSimulationEvent(FPySimGameEvent game_event);

	/* Schedule data update from simulation. */
	bool ScheduleBPAnimationDataUpdate(TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe> behaviour_data_update);


};

