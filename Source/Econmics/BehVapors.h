#pragma once

#include "BasicBehaviour.h"
#include "BehVapors.generated.h"

/* Holds binary information about whether or not the object is blooming */
USTRUCT(BlueprintType)
struct ECONMICS_API FPyBehVapors : public FPyBasicBehaviour {

	GENERATED_BODY()

public:

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	//float current_temperature;

	FPyBehVapors() : FPyBasicBehaviour() { };

};