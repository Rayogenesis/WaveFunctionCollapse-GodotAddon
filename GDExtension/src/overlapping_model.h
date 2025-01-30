#ifndef OVERLAPPING_MODEL_H
#define OVERLAPPING_MODEL_H

#include "model.h"

#include <unordered_map>

#include <godot_cpp/classes/tile_data.hpp>
#include <godot_cpp/classes/tile_set_atlas_source.hpp>


using namespace std;
namespace gd = godot;

class OverlappingModel : public Model {
	
private:
	struct TileChange {
		int16_t Base = -1;
		int16_t FlipH = -1;
		int16_t FlipV = -1;
		int16_t Rotate90 = -1;
		int16_t Rotate180 = -1;
		int16_t Rotate270 = -1;
		int16_t Rotate90FlipH = -1;
		int16_t Rotate90FlipV = -1;
	};

	struct TileProperties {
		uint16_t SourceID;
		uint16_t AtlasX;
		uint16_t AtlasY;
		uint16_t AlterID; // Se guardan sin ninguna transformación

		int16_t ChangeID = -1;

		bool patternFlipH = true;
		bool patternFlipV = true;
		bool patternRotate = true;

		bool tileFlipH = true;
		bool tileFlipV = true;
		bool tileRotate = true;

		bool changeFlipH = false;
		bool changeFlipV = false;
		bool changeRotate = false;
	};
	
	static const vector<int> transform;

	vector<TileProperties> tiles;
	vector<TileChange> changes;
	vector<vector<int16_t>> patterns;

	TileProperties groundProp;
	int16_t groundID;
	
	vector<int> colors; // Hay que borrarlo, es para que no pete de momento

	static vector<int16_t> makePattern(function<int16_t(int, int)> f, int N);
	static vector<int16_t> flipH(const vector<int16_t>& p, int N, const vector<TileProperties>& tp);
	static vector<int16_t> flipV(const vector<int16_t>& p, int N, const vector<TileProperties>& tp);
	static vector<int16_t> rotate(const vector<int16_t>& p, int N, const vector<TileProperties>& tp);

	static int64_t hash(const vector<int16_t>& p, int C);
	static bool agrees(const vector<int16_t>& p1, const vector<int16_t>& p2, int dx, int dy, int N);

	static int findChangeType(TileChange change, int16_t value);

	void Paint(gd::TileMapLayer* outputMap, int node, int patternID) override;
	void PaintFullPattern(gd::TileMapLayer* patternsMap, int nodeX, int nodeY, int patternID);

public:
	OverlappingModel(gd::TileMapLayer* sampleMap, gd::TileMapLayer* patternsMap, int N, gd::Ref<gd::RandomNumberGenerator> rngRef, int width, int height, bool periodicInput, bool periodic, bool ground, bool paintPatterns, Heuristic heuristic);
	void Save(const string& filename) override;
};

#endif