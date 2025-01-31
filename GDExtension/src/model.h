#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <stdexcept>
#include <unordered_set>

#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/mutex.hpp>

using namespace std;
namespace gd = godot;

class Model {
protected:
	struct pair_hash {
		template <typename T1, typename T2>
		size_t operator()(const pair<T1, T2>& p) const {
			// Combina los hashes de los dos elementos del par
			size_t h1 = hash<T1>{}(p.first);
			size_t h2 = hash<T2>{}(p.second);
			return h1 ^ (h2 << 1); // Mezcla los hashes de manera simple
		}
	};

	struct BacktrackStackItem {
		int nodeID;
		int patternID;

		int compDir0;
		int compDir1;
		int compDir2;
		int compDir3;
	};

	struct BacktrackData {
		int currentNode;
		list<BacktrackStackItem>::iterator lastItem;

		unordered_set<int> observationsMade;
		unordered_set<pair<int, int>, pair_hash> itemsStacked;
		vector<int> unobserveNodes;
	};

private:
	int NextUnobservedNode();
	int Observe(int node);
	bool Propagate();
	void Ban(int i, int t);
	void Clear(gd::Ref<gd::Mutex> mutex, gd::TileMapLayer* outputMap);
	
	int RandomDistProb(const vector<double>& weights, double rand);

protected:
	vector<vector<bool>> wave;
	vector<vector<vector<int>>> propagator;
	vector<vector<vector<int>>> compatible;
	vector<int> observed;

	vector<pair<int, int>> stack;
	int stackSize, observedSoFar;

	int outputWidth, outputHeight, patternsTotal, patternSize;
	bool periodic, ground;

	vector<double> weights;
	vector<double> weightLogWeights, distribution;

	vector<int> sumsOfOnes;
	double sumOfWeights, sumOfWeightLogWeights, startingEntropy;
	vector<double> sumsOfWeights, sumsOfWeightLogWeights, entropies;

	static const vector<int> directionX;
	static const vector<int> directionY;
	static const vector<int> opposite;
	
	bool notFilledWave;

	gd::Ref<gd::RandomNumberGenerator> rng;
	vector<int> groundPatterns;

	list<BacktrackStackItem> backtrackStack;
	list<BacktrackData> backtrackCheckpoints;
	BacktrackData currentCheckpoint;

	bool calculateDist, backtrack;

	virtual void Paint(gd::TileMapLayer* outputMap, int node, int patternID) = 0;

public:
	enum Heuristic { Entropy, MRV, Scanline };
	Heuristic heuristic;

	Model(gd::Ref<gd::RandomNumberGenerator> rngRef, int width, int height, int N, bool periodic, Heuristic heuristic);
	virtual ~Model() = default;

	void Init();
	bool Run(gd::Ref<gd::Mutex> mutex, gd::TileMapLayer* outputMap, int limit, bool countObservationAsFail, int decisionsWithoutBacktracking, bool realTimePainting); // Debería incluir el "Save". Si lo hacemos en otro lado, quitamos outputMap
	virtual void Save(const string& filename) = 0;

	int GetPatternSize() { return patternSize; }
};

#endif