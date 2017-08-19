#pragma once


#include "SimWorldInterface.h"
#include "EvStruct.h"
#include "Engine/StaticMeshActor.h"
#include "BaseBlock.generated.h"






/* A parent class for all the static mesh actors in the game. Linked to
an instance of FGameBlock ingame. */
UCLASS()
class ECONMICS_API ABaseBlock : public AStaticMeshActor
{
	GENERATED_BODY()

public:	

	ABaseBlock(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 gid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 cube_type;

	/* This call applies an event to internal values and then triggers BP event */
	void ApplyPySimGameEvent(FPySimGameEvent game_event);

	/* Blueprint event. Process animation of a simulated event here.*/
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Python simulation event"), Category = "Simulation glue")
	void python_simulation_new_event(FPySimGameEvent game_event);

	

};

