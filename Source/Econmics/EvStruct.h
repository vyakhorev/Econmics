#pragma once

#include "EvStruct.generated.h"

/* Send this structure to python_simulation_new_event event in actor's blueprint
in order to animate simulation events. */
USTRUCT(BlueprintType)
struct ECONMICS_API FPySimGameEvent {

	GENERATED_BODY()

public:

	/* Unique ID of a cube / tile / stack / planet - any simulated entity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 parent_gid;

	/* en.BehComponentRoles to route into a component and properly update data
	if needed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 behaviour_role;  

	FPySimGameEvent() {
		behaviour_role = 0;
		parent_gid = 0;
	};

};