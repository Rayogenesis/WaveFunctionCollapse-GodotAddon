#include "overlapping_model.h"

using namespace std;

OverlappingModel::OverlappingModel(gd::TileMapLayer* sampleMap, gd::TileMapLayer* patternsMap, int N, gd::Ref<gd::RandomNumberGenerator> rngRef, int width, int height, bool periodicInput, bool periodic, bool ground, bool paintPatterns, Heuristic heuristic)
    : Model(rngRef, width, height, N, periodic, heuristic)
{
    /*
        Antes cargábamos el bitmap, ahora sería solo obtener los datos del sample.
        Habría que mirar si hace falta el vector bitmap, o cómo tendríamos que adaptarlo.
        Probablemente bitmap debería ser un vector con todos los identificadores "traducidos" de los tiles.
    
    tuple<vector<int>, int, int> bitmap_tupple = LoadBitmap("samples/name.png");
    vector<int> bitmap = get<0>(bitmap_tupple);
    //int sampleWidth = get<1>(bitmap_tupple);
    //int sampleHeight = get<2>(bitmap_tupple);

    // Como sample tiene tamaño int16_t, entiendo que éste tiene identificadores de colores
    vector<int16_t> sample0(bitmap.size());
    // Colors tendrá todos los colores, mientras que sample tendrá en cada posición del bitmap el índice del color al que corresponde en colors
    colors.clear();
    // Por tanto, lo que nosotros necesitaremos solamente será sample. Bitmap sería el mapa original y colors el tileSet. Probablemente sample esté incluido en tileMapLayer, a ver si hay suerte

    // Convertimos todos los colores encontrados a índices (este paso ya no haría falta)
    for (size_t i = 0; i < sample0.size(); i++) {
        int color = bitmap[i];

        auto it = find(colors.begin(), colors.end(), color);
        if (it == colors.end()) {
            colors.push_back(color);
            sample0[i] = colors.size() - 1;
        }
        else {
            sample0[i] = distance(colors.begin(), it);
        }
    }
    */

    // Definimos el tamaño y creamos el sample inicializado a -1
    gd::Array usedCells = sampleMap->get_used_cells();
    gd::Vector2i cellPosition;
    int rightEdge = 0;
    int botEdge = 0;

    // i = index of cell position
    for (int i = 0; i < usedCells.size(); ++i) {
        cellPosition = usedCells[i];
        if (cellPosition.x > rightEdge) rightEdge = cellPosition.x;
        if (cellPosition.y > botEdge) botEdge = cellPosition.y;
    }
    int sampleWidth = rightEdge + 1;
    int sampleHeight = botEdge + 1;

    vector<int16_t> sample(sampleWidth * sampleHeight, -1);
    
    // Obtenemos todos los tiles usados y los añadimos al sample
    tiles.clear();
    vector<gd::TileData*> tileDatas;

    // Creamos el ground tile
    if (ground) {
        cellPosition = gd::Vector2i(0, botEdge);

        groundProp.SourceID = sampleMap->get_cell_source_id(cellPosition);
        gd::Vector2i atlasCoords = sampleMap->get_cell_atlas_coords(cellPosition);
        groundProp.AtlasX = atlasCoords.x;
        groundProp.AtlasY = atlasCoords.y;
        groundProp.AlterID = sampleMap->get_cell_alternative_tile(cellPosition);

        for (int x = 1; (x < sampleWidth) && ground; ++x) {
            cellPosition = gd::Vector2i(x, botEdge);
            TileProperties tileProp;
            tileProp.SourceID = sampleMap->get_cell_source_id(cellPosition);
            gd::Vector2i atlasCoords = sampleMap->get_cell_atlas_coords(cellPosition);
            tileProp.AtlasX = atlasCoords.x;
            tileProp.AtlasY = atlasCoords.y;
            tileProp.AlterID = sampleMap->get_cell_alternative_tile(cellPosition);

            ground = !(groundProp.SourceID != tileProp.SourceID || groundProp.AtlasX != tileProp.AtlasX || groundProp.AtlasY != tileProp.AtlasY || groundProp.AlterID != tileProp.AlterID);
        }
        if (ground) {
            bool groundFlippedH = groundProp.AlterID & gd::TileSetAtlasSource::TRANSFORM_FLIP_H;
            bool groundFlippedV = groundProp.AlterID & gd::TileSetAtlasSource::TRANSFORM_FLIP_V;
            bool groundTransposed = groundProp.AlterID & gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE;
            groundProp.AlterID = ((groundProp.AlterID & ~gd::TileSetAtlasSource::TRANSFORM_FLIP_H) & ~gd::TileSetAtlasSource::TRANSFORM_FLIP_V) & ~gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE;
            
            //gd::Vector3i customData = groundData->get_custom_data("Pattern: FlipH / FlipV / Rotate");
            groundProp.patternFlipH = -1; //!customData.x; Podríamos hacer que el patrón pudiese flip y después mirar si el groundtile puede flip tmb
            groundProp.patternFlipV = -1;
            groundProp.patternRotate = -1;
            
            groundID = -16 + 4 * groundTransposed + 2 * groundFlippedV + groundFlippedH;
            for (int x = 0; x < sampleWidth; ++x) sample[x + botEdge * sampleWidth] = groundID;
        }
    }

    // i = index of cell position
    for (int i = 0; i < usedCells.size(); ++i) {
        cellPosition = usedCells[i];
        if (cellPosition.x < 0 || cellPosition.x > rightEdge || cellPosition.y < 0 || cellPosition.y >= botEdge) continue;
        TileProperties tileProp;
        tileProp.SourceID = sampleMap->get_cell_source_id(cellPosition);
        gd::Vector2i atlasCoords = sampleMap->get_cell_atlas_coords(cellPosition);
        tileProp.AtlasX = atlasCoords.x;
        tileProp.AtlasY = atlasCoords.y;
        tileProp.AlterID = sampleMap->get_cell_alternative_tile(cellPosition);

        if (tileProp.SourceID < 0 || tileProp.AtlasX < 0 || tileProp.AtlasY < 0 || tileProp.AlterID < 0) {
            sample[cellPosition.x + cellPosition.y * sampleWidth] = -1;
            continue;
        }

        bool tileFlippedH = tileProp.AlterID & gd::TileSetAtlasSource::TRANSFORM_FLIP_H;
        bool tileFlippedV = tileProp.AlterID & gd::TileSetAtlasSource::TRANSFORM_FLIP_V;
        bool tileTransposed = tileProp.AlterID & gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE;
        tileProp.AlterID = ((tileProp.AlterID & ~gd::TileSetAtlasSource::TRANSFORM_FLIP_H) & ~gd::TileSetAtlasSource::TRANSFORM_FLIP_V) & ~gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE;
        
        gd::TileData* tileData = sampleMap->get_cell_tile_data(cellPosition);

        bool found = false;
        for (int j = 0; j < tiles.size() && !found; ++j) {
            if (tileProp.SourceID == tiles[j].SourceID && tileProp.AtlasX == tiles[j].AtlasX && tileProp.AtlasY == tiles[j].AtlasY && tileProp.AlterID == tiles[j].AlterID)
            {
                sample[cellPosition.x + cellPosition.y * sampleWidth] = 8 * j + 4 * tileTransposed + 2 * tileFlippedV + tileFlippedH;
                found = true;
            }
        }
        if (!found) {
            gd::Vector3i customData = tileData->get_custom_data("Pattern: FlipH / FlipV / Rotate");
            tileProp.patternFlipH = !customData.x;
            tileProp.patternFlipV = !customData.y;
            tileProp.patternRotate = !customData.z;
            
            if (tileProp.patternFlipH || tileProp.patternFlipV || tileProp.patternRotate) {
                customData = tileData->get_custom_data("Tile: FlipH / FlipV / Rotate");
                tileProp.tileFlipH = tileProp.patternFlipH && !customData.x;
                tileProp.tileFlipV = tileProp.patternFlipV && !customData.y;
                tileProp.tileRotate = tileProp.patternRotate && !customData.z;
            
                if (tileProp.tileFlipH || tileProp.tileFlipV || tileProp.tileRotate) {
                    customData = tileData->get_custom_data("Change: FlipH / FlipV / Rotate");
                    tileProp.changeFlipH = tileProp.tileFlipH && customData.x;
                    tileProp.changeFlipV = tileProp.tileFlipV && customData.y;
                    tileProp.changeRotate = tileProp.tileRotate && customData.z;
                }
            }
            sample[cellPosition.x + cellPosition.y * sampleWidth] = 8 * tiles.size() + 4 * tileTransposed + 2 * tileFlippedV + tileFlippedH;
            tiles.push_back(tileProp);
            tileDatas.push_back(tileData);
        }
    }

    // Obtenemos todos los changes
    changes.clear();
    // i = index of tile position
    for (int i = 0; i < tiles.size(); ++i) {
        TileProperties tileProp = tiles[i];
        gd::TileData* tileData = tileDatas[i];

        if (tileProp.ChangeID == -1 && (tileProp.changeFlipH || tileProp.changeFlipV || tileProp.changeRotate)) {
            TileChange tileChange;
            tileChange.Base = i;
            tileProp.ChangeID = changes.size();
            tiles[i] = tileProp;

            TileProperties changeProp;
            gd::Vector3i customData;
            int16_t changeX, changeY, changeAlt;
            bool found = false;
            
            if (tileProp.changeRotate) {
                customData = tileData->get_custom_data("Rotate 90º Change ID");
                changeX = customData.x;
                changeY = customData.y;
                changeAlt = customData.z;
            
                if (changeX != -1 && changeY != -1 && changeAlt != -1 &&
                    (changeX != tileProp.AtlasX || changeY != tileProp.AtlasY || changeAlt != tileProp.AlterID)) {
            
                    found = false;
                    for (int j = 0; j < tiles.size() && !found; ++j) {
                        changeProp = tiles[j];
                        if (tileProp.SourceID == changeProp.SourceID && changeX == changeProp.AtlasX && changeY == changeProp.AtlasY && changeAlt == changeProp.AlterID)
                        {
                            changeProp = tileProp;
                            changeProp.AtlasX = changeX;
                            changeProp.AtlasY = changeY;
                            changeProp.AlterID = changeAlt;
                            
                            tileChange.Rotate90 = j;
                            tiles[j] = changeProp;
                            found = true;
                        }
                    }
                    if (!found) {
                        changeProp = tileProp;
                        changeProp.AtlasX = changeX;
                        changeProp.AtlasY = changeY;
                        changeProp.AlterID = changeAlt;
                        
                        tileChange.Rotate90 = tiles.size();
                        tiles.push_back(changeProp);
                    }
                }

                customData = tileData->get_custom_data("Rotate 180º Change ID");
                changeX = customData.x;
                changeY = customData.y;
                changeAlt = customData.z;

                if (changeX != -1 && changeY != -1 && changeAlt != -1 &&
                    (changeX != tileProp.AtlasX || changeY != tileProp.AtlasY || changeAlt != tileProp.AlterID)) {

                    found = false;
                    for (int j = 0; j < tiles.size() && !found; ++j) {
                        changeProp = tiles[j];
                        if (tileProp.SourceID == changeProp.SourceID && changeX == changeProp.AtlasX && changeY == changeProp.AtlasY && changeAlt == changeProp.AlterID)
                        {
                            changeProp = tileProp;
                            changeProp.AtlasX = changeX;
                            changeProp.AtlasY = changeY;
                            changeProp.AlterID = changeAlt;

                            tileChange.Rotate180 = j;
                            tiles[j] = changeProp;
                            found = true;
                        }
                    }
                    if (!found) {
                        changeProp = tileProp;
                        changeProp.AtlasX = changeX;
                        changeProp.AtlasY = changeY;
                        changeProp.AlterID = changeAlt;

                        tileChange.Rotate180 = tiles.size();
                        tiles.push_back(changeProp);
                    }
                }

                customData = tileData->get_custom_data("Rotate 270º Change ID");
                changeX = customData.x;
                changeY = customData.y;
                changeAlt = customData.z;

                if (changeX != -1 && changeY != -1 && changeAlt != -1 &&
                    (changeX != tileProp.AtlasX || changeY != tileProp.AtlasY || changeAlt != tileProp.AlterID)) {

                    found = false;
                    for (int j = 0; j < tiles.size() && !found; ++j) {
                        changeProp = tiles[j];
                        if (tileProp.SourceID == changeProp.SourceID && changeX == changeProp.AtlasX && changeY == changeProp.AtlasY && changeAlt == changeProp.AlterID)
                        {
                            changeProp = tileProp;
                            changeProp.AtlasX = changeX;
                            changeProp.AtlasY = changeY;
                            changeProp.AlterID = changeAlt;

                            tileChange.Rotate270 = j;
                            tiles[j] = changeProp;
                            found = true;
                        }
                    }
                    if (!found) {
                        changeProp = tileProp;
                        changeProp.AtlasX = changeX;
                        changeProp.AtlasY = changeY;
                        changeProp.AlterID = changeAlt;

                        tileChange.Rotate270 = tiles.size();
                        tiles.push_back(changeProp);
                    }
                }

                if (tileProp.changeFlipH) {
                    customData = tileData->get_custom_data("Rotate 90º + FlipH Change ID");
                    changeX = customData.x;
                    changeY = customData.y;
                    changeAlt = customData.z;

                    if (changeX != -1 && changeY != -1 && changeAlt != -1 &&
                        (changeX != tileProp.AtlasX || changeY != tileProp.AtlasY || changeAlt != tileProp.AlterID)) {

                        found = false;
                        for (int j = 0; j < tiles.size() && !found; ++j) {
                            changeProp = tiles[j];
                            if (tileProp.SourceID == changeProp.SourceID && changeX == changeProp.AtlasX && changeY == changeProp.AtlasY && changeAlt == changeProp.AlterID)
                            {
                                changeProp = tileProp;
                                changeProp.AtlasX = changeX;
                                changeProp.AtlasY = changeY;
                                changeProp.AlterID = changeAlt;

                                tileChange.Rotate90FlipH = j;
                                tiles[j] = changeProp;
                                found = true;
                            }
                        }
                        if (!found) {
                            changeProp = tileProp;
                            changeProp.AtlasX = changeX;
                            changeProp.AtlasY = changeY;
                            changeProp.AlterID = changeAlt;

                            tileChange.Rotate90FlipH = tiles.size();
                            tiles.push_back(changeProp);
                        }
                    }
                }

                if (tileProp.changeFlipV) {
                    customData = tileData->get_custom_data("Rotate 90º + FlipV Change ID");
                    changeX = customData.x;
                    changeY = customData.y;
                    changeAlt = customData.z;

                    if (changeX != -1 && changeY != -1 && changeAlt != -1 &&
                        (changeX != tileProp.AtlasX || changeY != tileProp.AtlasY || changeAlt != tileProp.AlterID)) {

                        found = false;
                        for (int j = 0; j < tiles.size() && !found; ++j) {
                            changeProp = tiles[j];
                            if (tileProp.SourceID == changeProp.SourceID && changeX == changeProp.AtlasX && changeY == changeProp.AtlasY && changeAlt == changeProp.AlterID)
                            {
                                changeProp = tileProp;
                                changeProp.AtlasX = changeX;
                                changeProp.AtlasY = changeY;
                                changeProp.AlterID = changeAlt;

                                tileChange.Rotate90FlipV = j;
                                tiles[j] = changeProp;
                                found = true;
                            }
                        }
                        if (!found) {
                            changeProp = tileProp;
                            changeProp.AtlasX = changeX;
                            changeProp.AtlasY = changeY;
                            changeProp.AlterID = changeAlt;

                            tileChange.Rotate90FlipV = tiles.size();
                            tiles.push_back(changeProp);
                        }
                    }
                }
            }

            if (tileProp.changeFlipH) {
                customData = tileData->get_custom_data("FlipH Change ID");
                changeX = customData.x;
                changeY = customData.y;
                changeAlt = customData.z;

                if (changeX != -1 && changeY != -1 && changeAlt != -1 &&
                    (changeX != tileProp.AtlasX || changeY != tileProp.AtlasY || changeAlt != tileProp.AlterID)) {

                    found = false;
                    for (int j = 0; j < tiles.size() && !found; ++j) {
                        changeProp = tiles[j];
                        if (tileProp.SourceID == changeProp.SourceID && changeX == changeProp.AtlasX && changeY == changeProp.AtlasY && changeAlt == changeProp.AlterID)
                        {
                            changeProp = tileProp;
                            changeProp.AtlasX = changeX;
                            changeProp.AtlasY = changeY;
                            changeProp.AlterID = changeAlt;

                            tileChange.FlipH = j;
                            tiles[j] = changeProp;
                            found = true;
                        }
                    }
                    if (!found) {
                        changeProp = tileProp;
                        changeProp.AtlasX = changeX;
                        changeProp.AtlasY = changeY;
                        changeProp.AlterID = changeAlt;

                        tileChange.FlipH = tiles.size();
                        tiles.push_back(changeProp);
                    }
                }
            }

            if (tileProp.changeFlipV) {
                customData = tileData->get_custom_data("FlipV Change ID");
                changeX = customData.x;
                changeY = customData.y;
                changeAlt = customData.z;

                if (changeX != -1 && changeY != -1 && changeAlt != -1 &&
                    (changeX != tileProp.AtlasX || changeY != tileProp.AtlasY || changeAlt != tileProp.AlterID)) {

                    found = false;
                    for (int j = 0; j < tiles.size() && !found; ++j) {
                        changeProp = tiles[j];
                        if (tileProp.SourceID == changeProp.SourceID && changeX == changeProp.AtlasX && changeY == changeProp.AtlasY && changeAlt == changeProp.AlterID)
                        {
                            changeProp = tileProp;
                            changeProp.AtlasX = changeX;
                            changeProp.AtlasY = changeY;
                            changeProp.AlterID = changeAlt;

                            tileChange.FlipV = j;
                            tiles[j] = changeProp;
                            found = true;
                        }
                    }
                    if (!found) {
                        changeProp = tileProp;
                        changeProp.AtlasX = changeX;
                        changeProp.AtlasY = changeY;
                        changeProp.AlterID = changeAlt;

                        tileChange.FlipV = tiles.size();
                        tiles.push_back(changeProp);
                    }
                }
            }
            changes.push_back(tileChange);
        }
    }

    /*
        Aquí podríamos buscar rowSegments, para facilitar la búsqueda de patrones.
        Recorremos todo el sample. Nuestro primer nodo diferente de -1 será un posible leftVert.
        Si no tenemos patternSize nodos seguidos, no añadimos el leftVert. Si tenemos los suficientes,
        añadimos una pareja (leftVert, rightVert) y seguimos buscando el resto.
    */

    // Inicializamos las listas con los patrones, sus índices y sus pesos
    patterns.clear();
    groundPatterns.clear();
    unordered_map<int64_t, int> patternIndices;
    vector<double> weightList;

    // Definimos las posiciones de las que se extraerán los patrones
    int xmax = periodicInput ? sampleWidth : sampleWidth - patternSize + 1;
    int ymax = (periodicInput && !ground) ? sampleHeight : sampleHeight - patternSize + 1;

    for (int y = 0; y < ymax; ++y) {
        for (int x = 0; x < xmax; ++x) {
            // Creamos un vector con 8 patrones, 4 son para el original y sus rotaciones, otros 4 para cada reflejo
            vector<vector<int16_t>> allPatterns(8, vector<int16_t>());
            allPatterns[0] = makePattern([&](int dx, int dy) { return sample[(x + dx) % sampleWidth + (y + dy) % sampleHeight * sampleWidth]; }, patternSize);

            auto it0 = find(allPatterns[0].begin(), allPatterns[0].end(), -1);
            if (it0 != allPatterns[0].end()) continue;

            bool isGroundPattern = ground && (allPatterns[0][allPatterns[0].size() - 1] < -1);

            if (!isGroundPattern) {
                bool canFlipH = true, canFlipV = true, canRotate = true;
                for (int i = 0; i < allPatterns[0].size() && (canFlipH || canFlipV || canRotate); ++i) {
                    if (canFlipH) canFlipH = tiles[allPatterns[0][i] / 8].patternFlipH;
                    if (canFlipV) canFlipV = tiles[allPatterns[0][i] / 8].patternFlipV;
                    if (canRotate) canRotate = tiles[allPatterns[0][i] / 8].patternRotate;
                }

                if (canRotate) {
                    allPatterns[3] = rotate(allPatterns[0], patternSize, tiles);
                    allPatterns[4] = rotate(allPatterns[3], patternSize, tiles);
                    allPatterns[5] = rotate(allPatterns[4], patternSize, tiles);
                    if (canFlipH) allPatterns[6] = flipH(allPatterns[3], patternSize, tiles);
                    if (canFlipV) allPatterns[7] = flipV(allPatterns[3], patternSize, tiles);
                }
                if (canFlipH) allPatterns[1] = flipH(allPatterns[0], patternSize, tiles);
                if (canFlipV) allPatterns[2] = flipV(allPatterns[0], patternSize, tiles);
            }

            // Recorremos los niveles de simetría definidos (ahora ponemos 8)
            for (int k = 0; k < 8; ++k) {
                // Escogemos un patrón y creamos un hash
                vector<int16_t> currentPatern = allPatterns[k];
                if (currentPatern.size() < 1) continue;
                int64_t patternHash = hash(currentPatern, tiles.size());

                // Buscamos su índice para aumentar su peso o añadirlo
                auto it = patternIndices.find(patternHash);
                if (it != patternIndices.end()) {
                    weightList[it->second] += 1.0;
                }
                else {
                    patternIndices[patternHash] = weightList.size();
                    if (isGroundPattern) groundPatterns.push_back(weightList.size());

                    weightList.push_back(1.0);
                    patterns.push_back(currentPatern);
                }
            }
        }
    }

    // Asignamos la lista de pesos y la cantidad de patrones únicos encontrados
    weights = weightList;
    patternsTotal = weights.size();
    this->ground = ground;

    // Inicializamos el propagador con 4 direcciones
    propagator.resize(4);
    // d = direction
    for (int d = 0; d < 4; ++d) {
        // Cada dirección tiene un vector de tamaño patrones únicos
        propagator[d].resize(patternsTotal);
        // t = index of pattern
        for (int t1 = 0; t1 < patternsTotal; ++t1) {
            // Cada patrón contiene la lista de patrones compatibles gracias a la función agrees
            vector<int> compatiblePatterns;
            // t2 = index of pattern
            for (int t2 = 0; t2 < patternsTotal; ++t2) {
                if (agrees(patterns[t1], patterns[t2], directionX[d], directionY[d], patternSize))
                    compatiblePatterns.push_back(t2);
            }
            propagator[d][t1] = compatiblePatterns;
        }
    }

    if (paintPatterns) {
        int rows = gd::UtilityFunctions::floor(gd::UtilityFunctions::sqrt(patterns.size()));
        int cols = gd::UtilityFunctions::ceil(patterns.size() / rows);
        while (rows * cols < patterns.size()) ++rows;
    
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                int patternID = x + y * cols;
                if (patternID < patterns.size()) PaintFullPattern(patternsMap, x * (patternSize + 1), y * (patternSize + 1), patternID);
            }
        }
    }
}

// Recibe una función desde el original, rotate o reflect con la que procesará un patrón dado para devolverlo rotado o reflejado
vector<int16_t> OverlappingModel::makePattern(function<int16_t(int, int)> f, int N) {
    vector<int16_t> result(N * N);
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            result[x + y * N] = f(x, y);
        }
    }
    return result;
}

vector<int16_t> OverlappingModel::flipH(const vector<int16_t>& p, int N, const vector<TileProperties>& tp) {
    vector<int16_t> newPattern = p;
    for (int t = 0; t < newPattern.size(); ++t) {
        int tileID = newPattern[t] / 8;
        if (tp[tileID].tileFlipH) {
            if (newPattern[t] % 2 == 0) ++newPattern[t];
            else --newPattern[t];
        }
    }
    return makePattern([&](int x, int y) { return newPattern[N - 1 - x + y * N]; }, N);
}

vector<int16_t> OverlappingModel::flipV(const vector<int16_t>& p, int N, const vector<TileProperties>& tp) {
    vector<int16_t> newPattern = p;
    for (int t = 0; t < newPattern.size(); ++t) {
        int tileID = newPattern[t] / 8;
        if (tp[tileID].tileFlipV) {
            if ((newPattern[t] / 2) % 2 == 0) newPattern[t] += 2;
            else newPattern[t] -= 2;
        }
    }
    return makePattern([&](int x, int y) { return newPattern[x + (N - 1 - y) * N]; }, N);
}

const vector<int> OverlappingModel::transform = { 5, 0, 0, 6, 0, 3, 0, 0 };

vector<int16_t> OverlappingModel::rotate(const vector<int16_t>& p, int N, const vector<TileProperties>& tp) {
    vector<int16_t> newPattern = p;
    for (int t = 0; t < newPattern.size(); ++t) {
        int tileID = newPattern[t] / 8;
        if (tp[tileID].tileRotate) {
            newPattern[t] = 8 * tileID + transform[newPattern[t] % 8];
        }
    }
    return makePattern([&](int x, int y) { return newPattern[N - 1 - y + x * N]; }, N);
}

// Crea un hash para el patrón dado en base a la cantidad de colores únicos existentes
int64_t OverlappingModel::hash(const vector<int16_t>& p, int C) {
    int64_t result = 0;
    int64_t power = 1;
    for (size_t i = 0; i < p.size(); ++i) {
        result += p[p.size() - 1 - i] * power;
        power *= C;
    }
    return result;
}

// Comprueba si dos patrones son compatibles uno al lado del otro superponiéndose
bool OverlappingModel::agrees(const vector<int16_t>& p1, const vector<int16_t>& p2, int dx, int dy, int N) {
    int xmin = (dx < 0) ? 0 : dx;
    int xmax = (dx < 0) ? dx + N : N;
    int ymin = (dy < 0) ? 0 : dy;
    int ymax = (dy < 0) ? dy + N : N;
    for (int y = ymin; y < ymax; ++y) {
        for (int x = xmin; x < xmax; ++x) {
            int tileID1 = p1[x + N * y];
            int tileID2 = p2[x - dx + N * (y - dy)];



            if (tileID1 != tileID2) {
                return false;
            }
        }
    }
    return true;
}

int OverlappingModel::findChangeType(TileChange change, int16_t value) {
    if (change.Base == value) return 0;
    if (change.FlipH == value) return 1;
    if (change.FlipV == value) return 2;
    if (change.Rotate90 == value) return 3;
    if (change.Rotate180 == value) return 4;
    if (change.Rotate270 == value) return 5;
    if (change.Rotate90FlipH == value) return 6;
    if (change.Rotate90FlipV == value) return 7;
    return -1;
}

void OverlappingModel::Paint(gd::TileMapLayer* outputMap, int node, int patternID) {
    int xmax = 1;
    int ymax = 1;
    if (!periodic) {
        if (node % outputWidth == outputWidth - 1) xmax = patternSize;
        if (node / outputWidth == outputHeight - 1) ymax = patternSize;
    }

    for (int y = 0; y < ymax; ++y) {
        for (int x = 0; x < xmax; ++x) {
            int16_t tileID = patterns[patternID][x + y * patternSize];
            int tileTransform = tileID % 8;
            
            if (ground && tileID < -1) {
                outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), groundProp.SourceID, gd::Vector2i(groundProp.AtlasX, groundProp.AtlasY),
                    groundProp.AlterID + (tileTransform % 2) * gd::TileSetAtlasSource::TRANSFORM_FLIP_H + ((tileTransform / 2) % 2) * gd::TileSetAtlasSource::TRANSFORM_FLIP_V + ((tileTransform / 4) % 2) * gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE);
            }
            else if (tileID > -1) {
                tileID /= 8;
                TileProperties tileProp = tiles[tileID];
                TileChange tileChange;
                vector<int> changeTransform;

                if (tileTransform == 0) { // Base
                    outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                }

                else if (tileTransform == 1) { // H
                    if (tileProp.changeFlipH) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.FlipH, tileChange.Base, tileChange.Rotate180, tileChange.Rotate90FlipH,
                                            tileChange.FlipV, tileChange.Rotate90FlipV, tileChange.Rotate90, tileChange.Rotate270 };
                    
                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_H);
                    }
                    else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_H);
                }

                else if (tileTransform == 2) { // V
                    if (tileProp.changeFlipV) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.FlipV, tileChange.Rotate180, tileChange.Base, tileChange.Rotate90FlipV,
                                            tileChange.FlipH, tileChange.Rotate90FlipH, tileChange.Rotate270, tileChange.Rotate90 };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                    }
                    else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                }

                else if (tileTransform == 3) { // t + H
                    if (tileProp.changeRotate) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate90, tileChange.Rotate90FlipV, tileChange.Rotate90FlipH, tileChange.Rotate180,
                                            tileChange.Rotate270, tileChange.Base, tileChange.FlipH, tileChange.FlipV };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_H);
                    }
                    else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_H);
                }

                else if (tileTransform == 4) { // H + V
                    if (tileProp.changeRotate) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate180, tileChange.FlipV, tileChange.FlipH, tileChange.Rotate270,
                                            tileChange.Base, tileChange.Rotate90, tileChange.Rotate90FlipV, tileChange.Rotate90FlipH };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_H + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                    }
                    else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_H + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                }

                else if (tileTransform == 5) { // t + V
                    if (tileProp.changeRotate) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate270, tileChange.Rotate90FlipH, tileChange.Rotate90FlipV, tileChange.Base,
                                            tileChange.Rotate90, tileChange.Rotate180, tileChange.FlipV, tileChange.FlipH };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                    }
                    else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                }

                else if (tileTransform == 6) { // t
                    if (tileProp.changeRotate && tileProp.changeFlipH) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate90FlipH, tileChange.Rotate270, tileChange.Rotate90, tileChange.FlipV,
                                            tileChange.Rotate90FlipV, tileChange.FlipH, tileChange.Base, tileChange.Rotate180 };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE);
                    }
                    else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE);
                }

                else if (tileTransform == 7) { // t + H + V
                    if (tileProp.changeRotate && tileProp.changeFlipV) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate90FlipV, tileChange.Rotate90, tileChange.Rotate270, tileChange.FlipH,
                                            tileChange.Rotate90FlipH, tileChange.FlipV, tileChange.Rotate180, tileChange.Base };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_H + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                    }
                    else outputMap->call_deferred("set_cell", gd::Vector2i(node % outputWidth + x, node / outputWidth + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_H + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                }
            }
            //++tilesPainted;
        }
    }
}

void OverlappingModel::PaintFullPattern(gd::TileMapLayer* patternsMap, int nodeX, int nodeY, int patternID) {
    int xmax = patternSize;
    int ymax = patternSize;

    for (int y = 0; y < ymax; ++y) {
        for (int x = 0; x < xmax; ++x) {
            int16_t tileID = patterns[patternID][x + y * patternSize];
            int tileTransform = tileID % 8;
            if (ground && tileID < -1) {
                patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), groundProp.SourceID, gd::Vector2i(groundProp.AtlasX, groundProp.AtlasY),
                    groundProp.AlterID + (tileTransform % 2) * gd::TileSetAtlasSource::TRANSFORM_FLIP_H + ((tileTransform / 2) % 2) * gd::TileSetAtlasSource::TRANSFORM_FLIP_V + ((tileTransform / 4) % 2) * gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE);
            }
            else if (tileID > -1) {
                tileID /= 8;
                TileProperties tileProp = tiles[tileID];
                TileChange tileChange;
                vector<int> changeTransform;

                if (tileTransform == 0) { // Base
                    patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                }

                else if (tileTransform == 1) { // H
                    if (tileProp.changeFlipH) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.FlipH, tileChange.Base, tileChange.Rotate180, tileChange.Rotate90FlipH,
                                            tileChange.FlipV, tileChange.Rotate90FlipV, tileChange.Rotate90, tileChange.Rotate270 };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_H);
                    }
                    else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_H);
                }

                else if (tileTransform == 2) { // V
                    if (tileProp.changeFlipV) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.FlipV, tileChange.Rotate180, tileChange.Base, tileChange.Rotate90FlipV,
                                            tileChange.FlipH, tileChange.Rotate90FlipH, tileChange.Rotate270, tileChange.Rotate90 };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                    }
                    else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                }

                else if (tileTransform == 3) { // t + H
                    if (tileProp.changeRotate) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate90, tileChange.Rotate90FlipV, tileChange.Rotate90FlipH, tileChange.Rotate180,
                                            tileChange.Rotate270, tileChange.Base, tileChange.FlipH, tileChange.FlipV };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_H);
                    }
                    else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_H);
                }

                else if (tileTransform == 4) { // H + V
                    if (tileProp.changeRotate) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate180, tileChange.FlipV, tileChange.FlipH, tileChange.Rotate270,
                                            tileChange.Base, tileChange.Rotate90, tileChange.Rotate90FlipV, tileChange.Rotate90FlipH };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_H + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                    }
                    else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_FLIP_H + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                }

                else if (tileTransform == 5) { // t + V
                    if (tileProp.changeRotate) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate270, tileChange.Rotate90FlipH, tileChange.Rotate90FlipV, tileChange.Base,
                                            tileChange.Rotate90, tileChange.Rotate180, tileChange.FlipV, tileChange.FlipH };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                    }
                    else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                }

                else if (tileTransform == 6) { // t
                    if (tileProp.changeRotate && tileProp.changeFlipH) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate90FlipH, tileChange.Rotate270, tileChange.Rotate90, tileChange.FlipV,
                                            tileChange.Rotate90FlipV, tileChange.FlipH, tileChange.Base, tileChange.Rotate180 };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE);
                    }
                    else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE);
                }

                else if (tileTransform == 7) { // t + H + V
                    if (tileProp.changeRotate && tileProp.changeFlipV) {
                        tileChange = changes[tileProp.ChangeID];
                        changeTransform = { tileChange.Rotate90FlipV, tileChange.Rotate90, tileChange.Rotate270, tileChange.FlipH,
                                            tileChange.Rotate90FlipH, tileChange.FlipV, tileChange.Rotate180, tileChange.Base };

                        int16_t newTileID = changeTransform[findChangeType(tileChange, tileID)];
                        if (newTileID != -1) {
                            tileID = newTileID;
                            tileProp = tiles[tileID];
                            patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID);
                        }
                        else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_H + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                    }
                    else patternsMap->call_deferred("set_cell", gd::Vector2i(nodeX + x, nodeY + y), tileProp.SourceID, gd::Vector2i(tileProp.AtlasX, tileProp.AtlasY), tileProp.AlterID + gd::TileSetAtlasSource::TRANSFORM_TRANSPOSE + gd::TileSetAtlasSource::TRANSFORM_FLIP_H + gd::TileSetAtlasSource::TRANSFORM_FLIP_V);
                }
            }
        }
    }
}

void OverlappingModel::Save(const string& filename) {
    vector<int> bitmap(outputWidth * outputHeight, 0);

    if (observed[0] >= 0) {
        for (int y = 0; y < outputHeight; y++) {
            int dy = (y < outputHeight - patternSize + 1) ? 0 : patternSize - 1;
            for (int x = 0; x < outputWidth; x++) {
                int dx = (x < outputWidth - patternSize + 1) ? 0 : patternSize - 1;
                bitmap[x + y * outputWidth] = colors[patterns[observed[x - dx + (y - dy) * outputWidth]][dx + dy * patternSize]];
            }
        }
    }
    else {
        for (size_t i = 0; i < wave.size(); i++) {
            int contributors = 0, r = 0, g = 0, b = 0;
            int x = i % outputWidth;
            int y = i / outputWidth;
            for (int dy = 0; dy < patternSize; dy++) {
                for (int dx = 0; dx < patternSize; dx++) {
                    int sx = (x - dx + outputWidth) % outputWidth;
                    int sy = (y - dy + outputHeight) % outputHeight;
                    int s = sx + sy * outputWidth;
                    if (!periodic && (sx + patternSize > outputWidth || sy + patternSize > outputHeight || sx < 0 || sy < 0)) continue;
                    for (int t = 0; t < patternsTotal; t++) {
                        if (wave[s][t]) {
                            contributors++;
                            int argb = colors[patterns[t][dx + dy * patternSize]];
                            r += (argb & 0xff0000) >> 16;
                            g += (argb & 0xff00) >> 8;
                            b += argb & 0xff;
                        }
                    }
                }
            }
            bitmap[i] = (0xff000000 | ((r / contributors) << 16) | ((g / contributors) << 8) | (b / contributors));
        }
    }

    // SaveBitmap(bitmap, outputWidth, outputHeight, filename);
}