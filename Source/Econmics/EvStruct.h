#pragma once

#include "EvStruct.generated.h"

/* Send this structure to python_simulation_new_event event in actor's blueprint
in order to animate simulation events. */
USTRUCT(BlueprintType)
struct ECONMICS_API FGameEvent {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	FString event_description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	int32 parent_gid;

	FGameEvent() {
		event_description = "";
		parent_gid = 0;
	};

};