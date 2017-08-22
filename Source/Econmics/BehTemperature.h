#pragma once

#include "BasicBehaviour.h"
#include "BehTemperature.generated.h"

/* Holds binary information about whether or not the object is blooming. */
USTRUCT(BlueprintType)
struct ECONMICS_API FPyBehTemperature : public FPyBasicBehaviour {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float current_temperature;

	FPyBehTemperature() : FPyBasicBehaviour() {
		current_temperature = 0.0f;
	};

};