
#include "Econmics.h"
#include "SimWorldInterface.h"

/* ~~~~~~~~~~ Public ~~~~~~~~~~~~~~~~~~ */

/* SimWorldInterface - the master class for all simulation activities,
manages all the python calls */
SimWorldInterface::SimWorldInterface() {

	this->is_python_working = false;

	this->py_simulation_controller = NULL;
	this->py_ue4exec_module = NULL;

}

SimWorldInterface::~SimWorldInterface() {
	// Stop python in case for some reasons this
	// instance is deconstructed. However, it's
	// lifespan is the same as EconGameInstance.
	this->SafeStopPython();
}

bool SimWorldInterface::Init() {
	bool isok = true;
	isok &= this->SafeStartPython();
	isok &= this->ImportPyInterfaceModule();

	return isok;
}

bool SimWorldInterface::Shutdown() {
	if (this->is_python_working) {
		// Shall be stopped if we had an error
		Py_DECREF(this->py_simulation_controller);
		Py_DECREF(this->py_ue4exec_module);
	}
	// Stop Python
	return this->SafeStopPython();
}

TArray<SimGameBlock> SimWorldInterface::GetActiveChunk() {

	TArray<SimGameBlock> WorldArray;
	this->game_block_map.GenerateValueArray(WorldArray);
	return WorldArray;

}

bool SimWorldInterface::SpawnTestWorld() {

	static const FString repr("SimWorldInterface::spawnTestWorld");
	
	if (!this->is_python_working) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't spawn the world without python running"), *repr);
		return false;
	}

	PyObject *pControllerInitCall = PyObject_GetAttrString(this->py_ue4exec_module, "cSimulationController");
	if (!check_return_value(pControllerInitCall, "pControllerInitCall")) {
		// This can happen
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't get a callable for simulation controller creation"), *repr);
		this->SafeStopPython();
		return false;
	}

	PyObject *pController = PyObject_CallObject(pControllerInitCall, PyTuple_New(0));
	
	// ~~~ Save a link to the world instance
	this->py_simulation_controller = pController;
	Py_INCREF(this->py_simulation_controller);
	// ~~~

	if (!check_return_value(this->py_simulation_controller, "pController")) {
		// This can happen
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't construct the controller instance"), *repr);
		this->SafeStopPython();
		return false;
	}

	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with constructing controller instance"), *repr);
		this->SafeStopPython();
		return false;
	}

	// Spawn the world
	PyObject_CallMethod(this->py_simulation_controller, "generate_world", "()", NULL);
	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with spawning the world"), *repr);
		this->SafeStopPython();
		return false;
	}


	// Start loading the world (over all the chunks, so we'll have to change this later

	PyObject *preiterator = PyObject_CallMethod(this->py_simulation_controller, "iterate_over_blocks", "()", NULL);

	if (check_for_python_error() || preiterator == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over blocks"), *repr);
		this->SafeStopPython();
		return false;
	}


	PyObject *iterator_over_blocks = PyObject_GetIter(preiterator);

	if (check_for_python_error() || iterator_over_blocks == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over blocks"), *repr);
		this->SafeStopPython();
		return false;
	}

	PyObject *item = PyIter_Next(iterator_over_blocks);

	if (check_for_python_error() || item == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over blocks - before the loop"), *repr);
		this->SafeStopPython();
		return false;
	}

	while (item) {

		long x = PyLong_AsLong(PyObject_GetAttrString(item, "x"));
		if (check_for_python_error()) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with x of a block"), *repr);
			this->SafeStopPython();
			return false;
		};

		long y = PyLong_AsLong(PyObject_GetAttrString(item, "y"));
		if (check_for_python_error()) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with y of a block"), *repr);
			this->SafeStopPython();
			return false;
		};

		long z = PyLong_AsLong(PyObject_GetAttrString(item, "z"));
		if (check_for_python_error()) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with z of a block"), *repr);
			this->SafeStopPython();
			return false;
		};

		long gid = PyLong_AsLong(PyObject_GetAttrString(item, "gid"));
		if (check_for_python_error()) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with gid of a block"), *repr);
			this->SafeStopPython();
			return false;
		};

		long cube_type = PyLong_AsLong(PyObject_GetAttrString(item, "cube_type"));
		if (check_for_python_error()) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with cube_type of a block"), *repr);
			this->SafeStopPython();
			return false;
		};


		/* Populating the internal map */
		this->AddNewBlock(x, y, z, cube_type, gid);


		/* release reference when done */
		Py_DECREF(item);
		item = PyIter_Next(iterator_over_blocks);

		if (check_for_python_error()) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over blocks - inside the loop"), *repr);
			this->SafeStopPython();
			return false;
		};

	}

	//Py_DECREF(item);  // This call tried to access wrong memory and got the app crashed.
	Py_DECREF(iterator_over_blocks);

	return true;

}



// A private call when spawning the world
bool SimWorldInterface::AddNewBlock(long relX, long relY, long relZ, long cube_type, long gid) {

	/* A technical call to append to internal collection */

	SimGameBlock new_block = SimGameBlock();
	new_block.relX = relX;
	new_block.relY = relY;
	new_block.relZ = relZ;
	new_block.cube_type = cube_type;
	new_block.gid = gid;

	this->game_block_map.Add(gid, new_block);

	return true;

}


bool SimWorldInterface::RunSimulationInterval(float interval_tu) {

	static const FString repr("SimWorldInterface::RunSimulationInterval");

	if (this->py_simulation_controller == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't start simulation without controller"), *repr);
		return false;
	}

	if (!this->is_python_working) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't run simulation without python"), *repr);
		return false;
	}

	PyObject *pEventsList = PyObject_CallMethod(this->py_simulation_controller, "run_simulation_interval", "(f)", interval_tu);

	if (!check_return_value(pEventsList, "pEventsList")) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't get list of happened events"), *repr);
		this->SafeStopPython();
		return false;
	}

	/* Unpack list of events into internal variable */

	PyObject *iterator_over_events = PyObject_GetIter(pEventsList);

	if (check_for_python_error() || iterator_over_events == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over events"), *repr);
		this->SafeStopPython();
		return false;
	}

	PyObject *ev_i = PyIter_Next(iterator_over_events);

	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over events - before the loop"), *repr);
		this->SafeStopPython();
		return false;
	}

	this->recent_events.Empty();

	while (ev_i) {

		bool isok = this->RegisterRecentEvent(ev_i);

		if (!isok) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with event registration"), *repr);
			this->SafeStopPython();
			return false;
		}

		/* release reference when done */
		Py_DECREF(ev_i);
		ev_i = PyIter_Next(iterator_over_events);

		if (check_for_python_error()) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over events - inside the loop"), *repr);
			this->SafeStopPython();
			return false;
		};

	}

	Py_DECREF(iterator_over_events);

	return true;

}


bool SimWorldInterface::RegisterRecentEvent(PyObject *sim_event) {

	static const FString repr("SimWorldInterface::registerRecentEvent");

	PyObject *py_gid = PyObject_CallMethod(sim_event, "get_parent_gid", "()", NULL);
	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with get_parent_gid of an event"), *repr);
		this->SafeStopPython();
		return false;
	}

	PyObject *py_role = PyObject_CallMethod(sim_event, "get_behaviour_role", "()", NULL);
	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with get_behaviour_role of an event"), *repr);
		this->SafeStopPython();
		return false;
	}

	// Register which block 'received' an event
	int32 gid = PyLong_AsLong(py_gid);
	// Register which component (=behaviour) should be updated
	int32 role = PyLong_AsLong(py_role);

	FPySimGameEvent new_event = FPySimGameEvent();
	new_event.parent_gid = gid;
	new_event.behaviour_role = role;

	this->recent_events.Add(new_event);

	return true;

}


bool SimWorldInterface::GatherAnimationUpdates() {

	static const FString repr("SimWorldInterface::GatherAnimationUpdates");

	if (this->py_simulation_controller == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't work without controller"), *repr);
		return false;
	}

	if (!this->is_python_working) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't work without python"), *repr);
		return false;
	}

	PyObject *preiterator = PyObject_CallMethod(this->py_simulation_controller, "iterate_over_animation_updates", "()", NULL);

	if (check_for_python_error() || preiterator == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over animation updates"), *repr);
		this->SafeStopPython();
		return false;
	}

	PyObject *iterator_over_updates = PyObject_GetIter(preiterator);

	if (check_for_python_error() || iterator_over_updates == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over animation updates"), *repr);
		this->SafeStopPython();
		return false;
	}

	PyObject *item = PyIter_Next(iterator_over_updates);  //It will return NULL if there are no updates

	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over animation updates - before the loop"), *repr);
		this->SafeStopPython();
		return false;
	}

	// We buffer updates in PyThreadWorker in case the game thread didn't collect it yet.
	this->behaviour_updates.Empty();

	while (item) {

		/* Populate internal array */
		bool isok = this->RegisterAnimationUpdate(item);
		if (!isok) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with animation update registration"), *repr);
			this->SafeStopPython();
			return false;
		}

		/* release reference when done */
		Py_DECREF(item);
		item = PyIter_Next(iterator_over_updates);

		if (check_for_python_error()) {
			UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with iteration over updates - inside the loop"), *repr);
			this->SafeStopPython();
			return false;
		};

	}

	Py_DECREF(iterator_over_updates);

	return true;

}

bool SimWorldInterface::RegisterAnimationUpdate(PyObject *update_tuple) {

	static const FString repr("SimWorldInterface::RegisterAnimationUpdate");

	long gid = PyLong_AsLong(PyTuple_GetItem(update_tuple, 0));
	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with gid of animation update"), *repr);
		this->SafeStopPython();
		return false;
	};

	long role = PyLong_AsLong(PyTuple_GetItem(update_tuple, 1));
	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed with role of animation update"), *repr);
		this->SafeStopPython();
		return false;
	};

	if (role == 100) {
		// Construct object. This will let this pointer to own the object.
		// The structure shall be deleted when the pointer is out of scope
		// (it's passed all the way to the actor, deleted when new update
		// arrives, so it's important to have it thread safe since the
		// buffer array will be emptied on every tick).
		TSharedPtr<FPyBehBlooming, ESPMode::ThreadSafe> new_update(new FPyBehBlooming);
		new_update->parent_gid = gid;
		new_update->behaviour_role = role;

		new_update->is_blooming = true;  // TODO: read from python

		this->behaviour_updates.Add(new_update);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - failed to find role %d"), *repr, role);
		TSharedPtr<FPyBasicBehaviour, ESPMode::ThreadSafe> new_update(new FPyBasicBehaviour);
		new_update->parent_gid = gid;
		new_update->behaviour_role = role;

		this->behaviour_updates.Add(new_update);
	};
	
	return true;

}


/* ~~~~~~~~~~ Private ~~~~~~~~~~~~~~~~~~ */

bool SimWorldInterface::SafeStartPython() {

	static const FString repr("SimWorldInterface::safeStartPython");

	if (this->is_python_working) {
		UE_LOG(LogTemp, Log, TEXT("[%s] - python already loaded"), *repr);
		return true;
	}

	this->is_python_working = start_python();

	if (!this->is_python_working) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't start python!"), *repr);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] - python loaded"), *repr);

	return true;

}

bool SimWorldInterface::SafeStopPython() {

	static const FString repr("SimWorldInterface::safeStopPython");

	if (!this->is_python_working) {
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] - finalizing python execution..."), *repr);
	Py_Finalize();
	this->is_python_working = false;
	UE_LOG(LogTemp, Log, TEXT("[%s] - finalized python execution"), *repr);

	return true;

}

bool SimWorldInterface::ImportPyInterfaceModule() {

	static const FString repr("SimWorldInterface::importPyInterfaceModule");

	if (!this->is_python_working) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't import python module without python process"), *repr);
		return false;
	}

	/* So we hardcode ue4exec.py here. */
	PyObject *pName = PyUnicode_DecodeFSDefault("ue4exec");
	if (!check_return_value(pName, "pName")) {
		// This would barely happen..
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't decode module name"), *repr);
		this->SafeStopPython();
		return false;
	}

	PyObject *pModule = PyImport_Import(pName);
	if (!check_return_value(pName, "pModule")) {
		// This can happen
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't import the module"), *repr);
		Py_DECREF(pName);
		this->SafeStopPython();
		return false;
	}

	if (check_for_python_error()) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] - can't import the module - error"), *repr);
		this->SafeStopPython();
		return false;
	}
	Py_DECREF(pName);

	UE_LOG(LogTemp, Log, TEXT("[%s] - python module loaded"), *repr);

	// ~~~ Save a link to the module
	this->py_ue4exec_module = pModule;
	Py_INCREF(this->py_ue4exec_module);
	// ~~~

	return true;

}