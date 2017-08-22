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

