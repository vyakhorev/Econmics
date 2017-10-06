// This is a base class for all static mesh actors.

#include "Econmics.h"
#include "BaseBlock.h"


ABaseBlock::ABaseBlock(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	
}


void ABaseBlock::ApplyPyDataUpdate(TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe> behaviour_data_update) {

	// Abra cadabra sim sala bim! Get child object (the actual behaviour derived from FPyBasicBehaviour).
	// Akalay makalay muski mususki! For some reasons the Get returns a reference.
	// We have to hold an object (that is, copy the structure) in order to use it
	// in blue prints.

	if (behaviour_data_update->behaviour_role == 1) {
		auto beh = StaticCastSharedPtr<FPyBehTemperature, FPyBasicBehaviour, ESPMode::ThreadSafe>(behaviour_data_update);
		this->beh_temperature = (*beh.Get());
	}
	else if (behaviour_data_update->behaviour_role == 2) {
		auto beh = StaticCastSharedPtr<FPyBehVapors, FPyBasicBehaviour, ESPMode::ThreadSafe>(behaviour_data_update);
		this->beh_vapors = (*beh.Get());
	}
	else if (behaviour_data_update->behaviour_role == 3) {
		auto beh = StaticCastSharedPtr<FPyBehChemistry, FPyBasicBehaviour, ESPMode::ThreadSafe>(behaviour_data_update);
		this->beh_chemistry = (*beh.Get());
	}
	else if (behaviour_data_update->behaviour_role == 4) {
		auto beh = StaticCastSharedPtr<FPyBehBiomass, FPyBasicBehaviour, ESPMode::ThreadSafe>(behaviour_data_update);
		this->beh_biomass = (*beh.Get());
	}
	else if (behaviour_data_update->behaviour_role == 100) {
		auto beh = StaticCastSharedPtr<FPyBehBlooming, FPyBasicBehaviour, ESPMode::ThreadSafe>(behaviour_data_update);
		this->beh_blooming = (*beh.Get());
	}
	
	// Call general BP event
	this->python_simulation_data_update(behaviour_data_update->behaviour_role);

}

FString ABaseBlock::GetSimulationDataDescription() {
	// Returns a string with all simulation data. Helpful and simple.
	
	auto repr_gid = FString::Printf(TEXT("Block %d\n"), this->gid);

	auto repr_temp = FString::Printf(TEXT("Temperature = %.2f \n"), this->beh_temperature.current_temperature);

	auto repr_boom = FString::Printf(TEXT("Blooming = %d \n"), this->beh_blooming.is_blooming);

	auto repr_chem = FString::Printf(TEXT("Chemicals: C %.2f, Fe %.2f, K %.2f, O %.2f, H %.2f, N %.2f \n"),
									 this->beh_chemistry.perc_C,
								     this->beh_chemistry.perc_Fe,
									 this->beh_chemistry.perc_K,
									 this->beh_chemistry.perc_O,
								     this->beh_chemistry.perc_H,
								     this->beh_chemistry.perc_N);

	auto repr_biomass = FString::Printf(TEXT("Bio: bushes %.2f, grass %.2f, trees %.2f, predators %.2f, preys %.2f \n"),
										this->beh_biomass.bushes,
										this->beh_biomass.grass,
										this->beh_biomass.trees,
										this->beh_biomass.predators,
										this->beh_biomass.preys);

	auto ans = repr_gid + repr_temp + repr_chem + repr_biomass + repr_boom;
	
	return ans;

}

