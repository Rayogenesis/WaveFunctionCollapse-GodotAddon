#ifndef WFC_GENERATOR2D_H
#define WFC_GENERATOR2D_H

#include "model.h"
#include "overlapping_model.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

class WFCGenerator2D : public Node {
	GDCLASS(WFCGenerator2D, Node)

private:
	int patternSize, outputMapWidth, outputMapHeight, limitSteps, decisionsWithoutBacktracking;
	uint64_t executionSeed;
	bool periodicInput, periodicOutput, ground, isOverlapping, randomSeed, realTimePainting, paintPatterns, observationAsFail;
	String heuristic;

	TileMapLayer* sampleMap;
	TileMapLayer* patternsMap;
	TileMapLayer* outputMap;

	Ref<Thread> thread;
	Ref<Mutex> mutex;
	bool runningThread;

	Model* model;
	Ref<RandomNumberGenerator> rng;

	void start_running_thread();

protected:
	static void _bind_methods();

public:
	WFCGenerator2D();
	~WFCGenerator2D();

	bool build_model();
	bool delete_model();
	bool run_model();

	void finish_thread();

	// Getters
	int get_pattern_size() const;
	int get_output_map_width() const;
	int get_output_map_height() const;
	int get_limit_steps() const;
	int get_decisions_without_backtracking() const;

	uint64_t get_execution_seed() const;

	bool get_periodic_input() const;
	bool get_periodic_output() const;
	bool get_ground() const;
	bool get_random_seed() const;
	bool get_real_time_painting() const;
	bool get_paint_patterns() const;
	bool get_observation_as_fail() const;

	String get_heuristic() const;

	TileMapLayer* get_sample_map() const;
	TileMapLayer* get_patterns_map() const;
	TileMapLayer* get_output_map() const;

	// Setters
	void set_pattern_size(int input);
	void set_output_map_width(int input);
	void set_output_map_height(int input);
	void set_limit_steps(int input);
	void set_decisions_without_backtracking(int input);

	void set_execution_seed(uint64_t input);

	void set_periodic_input(bool input);
	void set_periodic_output(bool input);
	void set_ground(bool input);
	void set_random_seed(bool input);
	void set_real_time_painting(bool input);
	void set_paint_patterns(bool input);
	void set_observation_as_fail(bool input);

	void set_heuristic(String input);

	void set_sample_map(TileMapLayer* input);
	void set_patterns_map(TileMapLayer* input);
	void set_output_map(TileMapLayer* input);
};

#endif