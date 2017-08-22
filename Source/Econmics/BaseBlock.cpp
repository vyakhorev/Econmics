// This is a base class for all static mesh actors.

#include "Econmics.h"
#include "BaseBlock.h"


ABaseBlock::ABaseBlock(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	
}


void ABaseBlock::ApplyPySimGameEvent(FPySimGameEvent game_event) {

	// Now we're ready to process the blueprint animations
	this->python_simulation_new_event(game_event);

}

void ABaseBlock::ApplyPyDataUpdate(TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe> behaviour_data_update) {

	//UE_LOG(LogTemp, Warning, TEXT("Data update gid=%d role=%d "), behaviour_data_update->parent_gid, behaviour_data_update->behaviour_role);
	if (behaviour_data_update->behaviour_role == 100) {
		// Abra cadabra sim sala bim! Get child object (the actual behaviour derived from FPyBasicBehaviour).
		auto beh = StaticCastSharedPtr<FPyBehBlooming, FPyBasicBehaviour, ESPMode::ThreadSafe>(behaviour_data_update);
		// Akalay makalay muski mususki! For some reasons the Get returns a reference.
		// We have to hold an object (that is, copy the structure) in order to use it
		// in blue prints.
		this->beh_blooming = (*beh.Get());
		this->python_simulation_data_update(100);
	}
	

}

