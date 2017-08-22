#pragma once

#include "BasicBehaviour.h"
#include "BehChemistry.generated.h"

/* Holds binary information about whether or not the object is blooming */
USTRUCT(BlueprintType)
struct ECONMICS_API FPyBehChemistry : public FPyBasicBehaviour {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float perc_C;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float perc_Fe;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float perc_K;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float perc_O;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float perc_H;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	float perc_N;

	FPyBehChemistry() : FPyBasicBehaviour() {
		perc_C = 0.0f;
		perc_Fe = 0.0f;
		perc_K = 0.0f;
		perc_O = 0.0f;
		perc_H = 0.0f;
		perc_N = 0.0f;
	};

};