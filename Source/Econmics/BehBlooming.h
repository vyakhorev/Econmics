#pragma once

#include "BasicBehaviour.h"
#include "BehBlooming.generated.h"

/* Holds binary information about whether or not the object is blooming */
USTRUCT(BlueprintType)
struct ECONMICS_API FPyBehBlooming : public FPyBasicBehaviour {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation glue")
	bool is_blooming;

	FPyBehBlooming() : FPyBasicBehaviour() {
		is_blooming = false;
	};

};