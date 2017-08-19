// This is a base class for all static mesh actors.

#include "Econmics.h"
#include "BaseBlock.h"


ABaseBlock::ABaseBlock(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	
}


void ABaseBlock::ApplyPySimGameEvent(FPySimGameEvent game_event) {

	// 


	// Now we're ready to process the blueprint animations
	this->python_simulation_new_event(game_event);

}



