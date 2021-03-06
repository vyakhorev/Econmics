#pragma once

#include "BasicBehaviour.generated.h"

/* Holds binary information about whether or not the object is blooming */
USTRUCT(BlueprintType)
struct ECONMICS_API FPyBasicBehaviour {

	GENERATED_BODY()

public:

	/* Unique ID of a cube / tile / stack / planet - any simulated entity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 parent_gid;

	/* en.BehComponentRoles to route into a component and properly update data
	if needed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 behaviour_role;

	FPyBasicBehaviour() {
		behaviour_role = 0;
		parent_gid = 0;
	};

};