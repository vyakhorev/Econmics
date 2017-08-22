#pragma once


// I have to include behaviours directly, otherwise UBT fails
#include "BasicBehaviour.h"
#include "BehBlooming.h"
#include "BehTemperature.h"
#include "BehVapors.h"
#include "BehChemistry.h"
#include "BehBiomass.h"

#include "Engine/StaticMeshActor.h"
#include "BaseBlock.generated.h"


/* A parent class for all the static mesh actors in the game. Linked to
an instance of FGameBlock ingame. */
UCLASS()
class ECONMICS_API ABaseBlock : public AStaticMeshActor {

	GENERATED_BODY()

public:	

	ABaseBlock(const FObjectInitializer& ObjectInitializer);

	/* Block's state is defined with numeric values of different simulation
	aspects called behaviours. So these slots replicate python data. */

	/* Temperature of a block in F */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	FPyBehTemperature beh_temperature;
	/* Not implemented yet */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	FPyBehVapors beh_vapors;
	/* Radicals in block, this is soil fertility model */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	FPyBehChemistry beh_chemistry;
	/* Everything that lives */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	FPyBehBiomass beh_biomass;
	/* Testing swithing state from blooming into non-blooming */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	FPyBehBlooming beh_blooming;

	/* Unique code for each simulated entity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 gid;

	/* Pythond enumeration of basic elements */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 cube_type;

	/* This call updates simulation data inside this actor. The updates come from 
	another thread, that's why the declaration is so long. */
	void ApplyPyDataUpdate(TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe> behaviour_data_update);

	/* Blueprint event. Data update notification */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Python data update"), Category = "Simulation glue")
	void python_simulation_data_update(int32 behaviour_role);

};

