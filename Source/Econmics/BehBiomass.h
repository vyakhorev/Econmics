#pragma once

#include "BasicBehaviour.h"
#include "BehBiomass.generated.h"

/* Holds binary information about whether or not the object is blooming */
USTRUCT(BlueprintType)
struct ECONMICS_API FPyBehBiomass : public FPyBasicBehaviour {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float bushes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float grass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float trees;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float predators;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float preys;

	FPyBehBiomass() : FPyBasicBehaviour() {
		bushes = 0.0f;
		grass = 0.0f;
		trees = 0.0f;
		predators = 0.0f;
		preys = 0.0f;
	};

};