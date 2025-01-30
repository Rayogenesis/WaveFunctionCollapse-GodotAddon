#include "wfc_generator2d.h"

using namespace godot;

void WFCGenerator2D::_bind_methods() {

	// Getters and Setters
	ClassDB::bind_method(D_METHOD("get_pattern_size"), &WFCGenerator2D::get_pattern_size);
	ClassDB::bind_method(D_METHOD("set_pattern_size", "i_pattern_size"), &WFCGenerator2D::set_pattern_size);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "pattern_size", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "NxN size for patterns"), "set_pattern_size", "get_pattern_size");
	
	ClassDB::bind_method(D_METHOD("get_output_map_width"), &WFCGenerator2D::get_output_map_width);
	ClassDB::bind_method(D_METHOD("set_output_map_width", "i_output_map_width"), &WFCGenerator2D::set_output_map_width);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "output_map_width", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "Number of tiles in the output's X axis"), "set_output_map_width", "get_output_map_width");
	
	ClassDB::bind_method(D_METHOD("get_output_map_height"), &WFCGenerator2D::get_output_map_height);
	ClassDB::bind_method(D_METHOD("set_output_map_height", "i_output_map_height"), &WFCGenerator2D::set_output_map_height);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "output_map_height", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "Number of tiles in the output's Y axis"), "set_output_map_height", "get_output_map_height");
	
	ClassDB::bind_method(D_METHOD("get_limit_steps"), &WFCGenerator2D::get_limit_steps);
	ClassDB::bind_method(D_METHOD("set_limit_steps", "i_limit_steps"), &WFCGenerator2D::set_limit_steps);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "limit_steps", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "Max number of possible backsteps"), "set_limit_steps", "get_limit_steps");
	
	ClassDB::bind_method(D_METHOD("get_execution_seed"), &WFCGenerator2D::get_execution_seed);
	ClassDB::bind_method(D_METHOD("set_execution_seed", "i_execution_seed"), &WFCGenerator2D::set_execution_seed);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "execution_seed", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "Seed used for generating random numbers"), "set_execution_seed", "get_execution_seed");

	ClassDB::bind_method(D_METHOD("get_decisions_without_backtracking"), &WFCGenerator2D::get_decisions_without_backtracking);
	ClassDB::bind_method(D_METHOD("set_decisions_without_backtracking", "i_decisions_without_backtracking"), &WFCGenerator2D::set_decisions_without_backtracking);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "decisions_without_backtracking", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "Number of observations that won't save information for backtracking, thus making the process faster and unsafer at the start"), "set_decisions_without_backtracking", "get_decisions_without_backtracking");
	
	ClassDB::bind_method(D_METHOD("get_periodic_input"), &WFCGenerator2D::get_periodic_input);
	ClassDB::bind_method(D_METHOD("set_periodic_input", "i_periodic_input"), &WFCGenerator2D::set_periodic_input);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "periodic_input", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "If true, tiles will be adjacent across the input map's edges and will be able to form patterns"), "set_periodic_input", "get_periodic_input");
	
	ClassDB::bind_method(D_METHOD("get_periodic_output"), &WFCGenerator2D::get_periodic_output);
	ClassDB::bind_method(D_METHOD("set_periodic_output", "i_periodic_output"), &WFCGenerator2D::set_periodic_output);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "periodic_output", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "If true, tiles will follow adjacency rules across the output map's edges"), "set_periodic_output", "get_periodic_output");
	
	ClassDB::bind_method(D_METHOD("get_ground"), &WFCGenerator2D::get_ground);
	ClassDB::bind_method(D_METHOD("set_ground", "i_ground"), &WFCGenerator2D::set_ground);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ground", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "If true, the bottom tile from input will also be set at the ouput's bottom"), "set_ground", "get_ground");
	
	ClassDB::bind_method(D_METHOD("get_random_seed"), &WFCGenerator2D::get_random_seed);
	ClassDB::bind_method(D_METHOD("set_random_seed", "i_random_seed"), &WFCGenerator2D::set_random_seed);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "random_seed", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "Ignore execution seed and choose one randomly"), "set_random_seed", "get_random_seed");

	ClassDB::bind_method(D_METHOD("get_real_time_painting"), &WFCGenerator2D::get_real_time_painting);
	ClassDB::bind_method(D_METHOD("set_real_time_painting", "i_real_time_painting"), &WFCGenerator2D::set_real_time_painting);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "real_time_painting", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "Output map's tiles will be painted in real time"), "set_real_time_painting", "get_real_time_painting");
	
	ClassDB::bind_method(D_METHOD("get_paint_patterns"), &WFCGenerator2D::get_paint_patterns);
	ClassDB::bind_method(D_METHOD("set_paint_patterns", "i_paint_patterns"), &WFCGenerator2D::set_paint_patterns);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "paint_patterns", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "Paint all patterns found"), "set_paint_patterns", "get_paint_patterns");

	ClassDB::bind_method(D_METHOD("get_observation_as_fail"), &WFCGenerator2D::get_observation_as_fail);
	ClassDB::bind_method(D_METHOD("set_observation_as_fail", "i_observation_as_fail"), &WFCGenerator2D::set_observation_as_fail);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "observation_as_fail", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "If false, only backsteps will count as fails"), "set_observation_as_fail", "get_observation_as_fail");

	ClassDB::bind_method(D_METHOD("get_heuristic"), &WFCGenerator2D::get_heuristic);
	ClassDB::bind_method(D_METHOD("set_heuristic", "i_heuristic"), &WFCGenerator2D::set_heuristic);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "heuristic", PROPERTY_HINT_ENUM, "Entropy,MRV", PROPERTY_USAGE_DEFAULT, "Choose which heuristic will the algorithm follow"), "set_heuristic", "get_heuristic"); // Deleted Scanline due to incompatibilities with backtracking
	
	ClassDB::bind_method(D_METHOD("get_sample_map"), &WFCGenerator2D::get_sample_map);
	ClassDB::bind_method(D_METHOD("set_sample_map", "i_sample_map"), &WFCGenerator2D::set_sample_map);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sample_map", PROPERTY_HINT_NODE_TYPE, "TileMapLayer", PROPERTY_USAGE_DEFAULT, "This map will be used as an example for the algorithm"), "set_sample_map", "get_sample_map");

	ClassDB::bind_method(D_METHOD("get_patterns_map"), &WFCGenerator2D::get_patterns_map);
	ClassDB::bind_method(D_METHOD("set_patterns_map", "i_patterns_map"), &WFCGenerator2D::set_patterns_map);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "patterns_map", PROPERTY_HINT_NODE_TYPE, "TileMapLayer", PROPERTY_USAGE_DEFAULT, "This is the map where patterns will be painted if paint_patterns == true"), "set_patterns_map", "get_patterns_map");

	ClassDB::bind_method(D_METHOD("get_output_map"), &WFCGenerator2D::get_output_map);
	ClassDB::bind_method(D_METHOD("set_output_map", "i_output_map"), &WFCGenerator2D::set_output_map);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "output_map", PROPERTY_HINT_NODE_TYPE, "TileMapLayer", PROPERTY_USAGE_DEFAULT, "This is the map where the output will be generated"), "set_output_map", "get_output_map");

	// Methods
	ClassDB::bind_method(D_METHOD("build_model"), &WFCGenerator2D::build_model);
	ClassDB::bind_method(D_METHOD("delete_model"), &WFCGenerator2D::delete_model);
	ClassDB::bind_method(D_METHOD("run_model"), &WFCGenerator2D::run_model);
	ClassDB::bind_method(D_METHOD("start_running_thread"), &WFCGenerator2D::start_running_thread);
	ClassDB::bind_method(D_METHOD("finish_thread"), &WFCGenerator2D::finish_thread);

	// Signals
	ADD_SIGNAL(MethodInfo("wfc_finished_running"));
}

WFCGenerator2D::WFCGenerator2D() {
	patternSize = outputMapWidth = outputMapHeight = 0;
	limitSteps = decisionsWithoutBacktracking = -1;
	executionSeed = 42; // Seed initialization
	periodicInput = periodicOutput = ground = isOverlapping = randomSeed = observationAsFail = runningThread = false;
	realTimePainting = paintPatterns = true;
	heuristic = "Entropy";
	model = nullptr;
	sampleMap = nullptr;
	patternsMap = nullptr;
	outputMap = nullptr;
	mutex.reference_ptr(memnew(Mutex));
	rng.reference_ptr(memnew(RandomNumberGenerator));
}

WFCGenerator2D::~WFCGenerator2D() {
	// Add your cleanup here.
	if (model != nullptr) memdelete(model);
	thread.unref();
	mutex.unref();
	rng.unref();
}

bool WFCGenerator2D::build_model() {
	// Early Return conditions
	if (model != nullptr) return false; // Already have a model built
	if (sampleMap == nullptr || (patternsMap == nullptr && paintPatterns)) return false; // Sample or patterns (if necessary) map is null
	if (patternSize < 2 || patternSize > outputMapWidth || patternSize > outputMapHeight) return false; // Incorrect pattern size

	// Set the correct ouput sizes
	if (!periodicOutput) {
		outputMapWidth -= patternSize - 1;
		outputMapHeight -= patternSize - 1;
	}
	
	// Set the execution seed
	if (randomSeed) {
		rng->randomize();
		executionSeed = rng->get_seed();
	}
	executionSeed %= 9007199254640993; // Para tener un rango de casi 2^53
	rng->set_seed(executionSeed);
	
	isOverlapping = patternSize > 1;
	Model::Heuristic modelHeuristic = /*(heuristic == "Scanline") ? Model::Heuristic::Scanline :*/
		(heuristic == "MRV") ? Model::Heuristic::MRV :
		Model::Heuristic::Entropy;
	
	if (isOverlapping)
	{
		model = memnew(OverlappingModel(sampleMap, patternsMap, patternSize, rng, outputMapWidth, outputMapHeight, periodicInput, periodicOutput, ground, paintPatterns, modelHeuristic));
	}
	return true;
}

bool WFCGenerator2D::delete_model() {
	if (model != nullptr) {
		memdelete(model);
		return true;
	}
	return false;
}

bool WFCGenerator2D::run_model() {
	// Early Return conditions
	if (outputMap == nullptr || model == nullptr) return false; // Output map or the model is null
	if (runningThread) return false; // Thread already running
	runningThread = true;

	// Iniciar el hilo
	thread.reference_ptr(memnew(Thread));
	thread->start(callable_mp(this, &WFCGenerator2D::start_running_thread), Thread::PRIORITY_NORMAL);

	return true;
}

void WFCGenerator2D::start_running_thread() {
	if (model->Run(mutex, outputMap, limitSteps, observationAsFail, decisionsWithoutBacktracking, realTimePainting))
	{
		UtilityFunctions::print("DONE");
	}
	else
	{
		UtilityFunctions::print("CONTRADICTION");
	}
	runningThread = false;
	call_deferred("emit_signal", "wfc_finished_running");
}

void WFCGenerator2D::finish_thread() {
	if (thread.is_valid() && thread->is_alive()) {
		thread->wait_to_finish();
	}
	thread->unreference();
}

// Getters
int WFCGenerator2D::get_pattern_size() const {
	return patternSize;
}

int WFCGenerator2D::get_output_map_width() const {
	return outputMapWidth;
}

int WFCGenerator2D::get_output_map_height() const {
	return outputMapHeight;
}

int WFCGenerator2D::get_limit_steps() const {
	return limitSteps;
}

int WFCGenerator2D::get_decisions_without_backtracking() const {
	return decisionsWithoutBacktracking;
}

uint64_t WFCGenerator2D::get_execution_seed() const {
	return executionSeed;
}

bool WFCGenerator2D::get_periodic_input() const {
	return periodicInput;
}

bool WFCGenerator2D::get_periodic_output() const {
	return periodicOutput;
}

bool WFCGenerator2D::get_ground() const {
	return ground;
}

bool WFCGenerator2D::get_random_seed() const {
	return randomSeed;
}

bool WFCGenerator2D::get_real_time_painting() const {
	return realTimePainting;
}

bool WFCGenerator2D::get_paint_patterns() const {
	return paintPatterns;
}

bool WFCGenerator2D::get_observation_as_fail() const {
	return observationAsFail;
}

String WFCGenerator2D::get_heuristic() const {
	return heuristic;
}

TileMapLayer* WFCGenerator2D::get_sample_map() const {
	return sampleMap;
}

TileMapLayer* WFCGenerator2D::get_patterns_map() const {
	return patternsMap;
}

TileMapLayer* WFCGenerator2D::get_output_map() const {
	return outputMap;
}

// Setters
void WFCGenerator2D::set_pattern_size(int input) {
	patternSize = input;
}

void WFCGenerator2D::set_output_map_width(int input) {
	outputMapWidth = input;
}

void WFCGenerator2D::set_output_map_height(int input) {
	outputMapHeight = input;
}

void WFCGenerator2D::set_limit_steps(int input) {
	limitSteps = input;
}

void WFCGenerator2D::set_decisions_without_backtracking(int input) {
	decisionsWithoutBacktracking = input;
}

void WFCGenerator2D::set_execution_seed(uint64_t input) {
	executionSeed = input;
}

void WFCGenerator2D::set_periodic_input(bool input) {
	periodicInput = input;
}

void WFCGenerator2D::set_periodic_output(bool input) {
	periodicOutput = input;
}

void WFCGenerator2D::set_ground(bool input) {
	ground = input;
}

void WFCGenerator2D::set_random_seed(bool input) {
	randomSeed = input;
}

void WFCGenerator2D::set_real_time_painting(bool input) {
	realTimePainting = input;
}

void WFCGenerator2D::set_paint_patterns(bool input) {
	paintPatterns = input;
}

void WFCGenerator2D::set_observation_as_fail(bool input) {
	observationAsFail = input;
}

void WFCGenerator2D::set_heuristic(String input) {
	heuristic = input;
}

void WFCGenerator2D::set_sample_map(TileMapLayer* input) {
	if (input == nullptr) {
		sampleMap = input;
		return;
	}
	if (!input->is_class("TileMapLayer")) {
		UtilityFunctions::printerr("Assigned object is not a TileMapLayer!");
		return;
	}
	sampleMap = input;

	Ref<TileSet> tileset = sampleMap->get_tile_set();

	int count;
	if (tileset->get_custom_data_layer_by_name("Pattern: FlipH / FlipV / Rotate") == -1)
	{
		// El patrón puede X
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "Pattern: FlipH / FlipV / Rotate");
	}
	if (tileset->get_custom_data_layer_by_name("Tile: FlipH / FlipV / Rotate") == -1)
	{
		// Si el patrón puede X, el tile puede X
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "Tile: FlipH / FlipV / Rotate");
	}
	if (tileset->get_custom_data_layer_by_name("Change: FlipH / FlipV / Rotate") == -1)
	{
		// Si el tile puede X, hace falta cambiar
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "Change: FlipH / FlipV / Rotate");
	}
	if (tileset->get_custom_data_layer_by_name("FlipH Change ID") == -1)
	{
		// H
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "FlipH Change ID");
	}
	if (tileset->get_custom_data_layer_by_name("FlipV Change ID") == -1)
	{
		// V
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "FlipV Change ID");
	}
	if (tileset->get_custom_data_layer_by_name("Rotate 90º Change ID") == -1)
	{
		// t + H
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "Rotate 90º Change ID");
	}
	if (tileset->get_custom_data_layer_by_name("Rotate 180º Change ID") == -1)
	{
		// H + V
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "Rotate 180º Change ID");
	}
	if (tileset->get_custom_data_layer_by_name("Rotate 270º Change ID") == -1)
	{
		// t + V
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "Rotate 270º Change ID");
	}
	if (tileset->get_custom_data_layer_by_name("Rotate 90º + FlipH Change ID") == -1)
	{
		// t
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "Rotate 90º + FlipH Change ID");
	}
	if (tileset->get_custom_data_layer_by_name("Rotate 90º + FlipV Change ID") == -1)
	{
		// t + H + V
		count = tileset->get_custom_data_layers_count();
		tileset->add_custom_data_layer();
		tileset->set_custom_data_layer_type(count, Variant::VECTOR3I);
		tileset->set_custom_data_layer_name(count, "Rotate 90º + FlipV Change ID");
	}
}

void WFCGenerator2D::set_patterns_map(TileMapLayer* input) {
	patternsMap = input;
}

void WFCGenerator2D::set_output_map(TileMapLayer* input) {
	outputMap = input;
}