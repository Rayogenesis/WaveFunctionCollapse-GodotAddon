#include "model.h"

using namespace std;
namespace gd = godot;

const vector<int> Model::directionX = { -1, 0, 1, 0 };
const vector<int> Model::directionY = { 0, 1, 0, -1 };
const vector<int> Model::opposite = { 2, 3, 0, 1 };

Model::Model(gd::Ref<gd::RandomNumberGenerator> rngRef, int width, int height, int N, bool periodic, Heuristic heuristic)
    : rng(rngRef), outputWidth(width), outputHeight(height), patternSize(N), periodic(periodic), heuristic(heuristic),
    stackSize(0), observedSoFar(0), notFilledWave(true) {}

/*
    Inicializamos varias estructuras de datos que almacenan el estado actual del colapso de la función de onda.
    Calculamos los pesos y las entropías iniciales de cada celda.
    Nada de Godot por hacer.
*/
void Model::Init() {
    notFilledWave = false;
    wave.resize(outputWidth * outputHeight, vector<bool>(patternsTotal, true)); // Wave tamaño output y en cada celda todos los patrones posibles inicializados como válidos
    compatible.resize(outputWidth * outputHeight, vector<vector<int>>(patternsTotal, vector<int>(4, 0))); // Compatible tamaño output, en cada celda todos los patrones posibles inicializados con un vector tamaño cuatro inicializado a 0
    distribution.resize(patternsTotal); // Distribution tamaño patrones posibles sin inicializar
    observed.resize(outputWidth * outputHeight, -1); // Observed tamaño output inicializado a -1

    weightLogWeights.resize(patternsTotal); // ... tamaño patrones posibles sin inicializar
    sumOfWeights = 0;
    sumOfWeightLogWeights = 0;

    // t = index of pattern
    for (int t = 0; t < patternsTotal; ++t) { // Por cada posible patrón
        weightLogWeights[t] = weights[t] * log(weights[t]); // Calculamos weLoWe
        sumOfWeights += weights[t]; // Añadimos a la suma de pesos
        sumOfWeightLogWeights += weightLogWeights[t]; // Añadimos a la suma de Log pesos
    }

    startingEntropy = log(sumOfWeights) - sumOfWeightLogWeights / sumOfWeights; // Calculamos entropía inicial

    sumsOfOnes.resize(outputWidth * outputHeight); // Suma de unos tamaño output sin inicializar
    sumsOfWeights.resize(outputWidth * outputHeight); // Sumas de pesos tamaño output sin inicializar
    sumsOfWeightLogWeights.resize(outputWidth * outputHeight); // Sumas de weLoWe tamaño output sin inicializar
    entropies.resize(outputWidth * outputHeight); // Entropías tamaño output sin inicializar

    stack.resize(wave.size() * patternsTotal); // Stack tamaño output mult la cantidad de patrones
    stackSize = 0;
}

bool Model::Run(gd::Ref<gd::Mutex> mutex, gd::TileMapLayer* outputMap, int limit, bool countObservationAsFail, int decisionsWithoutBacktracking, bool realTimePainting) {
    // Inicializamos el modelo si no lo está ya
    if (wave.empty()) Init();
    
    // Reiniciamos el tablero (no tiene backtracking)
    Clear(mutex, outputMap);

    bool success = true;
    if (ground) {
        success = Propagate();

        if (realTimePainting || !success) {
            // i = index of wave node
            for (int i = 0; i < wave.size(); ++i)
            {
                if (sumsOfOnes[i] == 1 && observed[i] == -1) {
                    // t = index of pattern
                    for (int t = 0; t < patternsTotal; ++t)
                    {
                        if (wave[i][t]) {
                            observed[i] = t;
                            mutex->lock();
                            Paint(outputMap, i, t);
                            mutex->unlock();
                            break;
                        }
                    }
                }
            }
        }
        if (!success) {
            return false;
        }
    }
    int deccc = 0;
    int fails = 0;
    int decisions = 0;

    while (fails <= limit || limit < 0) {
        int node = NextUnobservedNode();
        backtrack = (decisions >= decisionsWithoutBacktracking) && (decisionsWithoutBacktracking != -1);
        if (!backtrack) {
            if (decisionsWithoutBacktracking != -1) ++decisions;

            if (node >= 0) {
                // Una vez seleccionado un nodo, guardamos datos para poder hacer backtracking a éste. Si probamos todas las posibles observaciones y no dan resultado,
                // tendríamos que hacer backtrack hacia el anterior nodo seleccionado y probar otras observaciones
                success = false;
                int observedPattern = Observe(node);
                success = Propagate();

                if (realTimePainting || !success) {
                    // i = index of wave node
                    for (int i = 0; i < wave.size(); ++i)
                    {
                        if (sumsOfOnes[i] == 1 && observed[i] == -1) {
                            // t = index of pattern
                            for (int t = 0; t < patternsTotal; ++t)
                            {
                                if (wave[i][t]) {
                                    observed[i] = t;
                                    mutex->lock();
                                    Paint(outputMap, i, t);
                                    mutex->unlock();
                                    break;
                                }
                            }
                        }
                    }
                }
                if (!success) {
                    return false;
                }

                if (success) {
                    if (realTimePainting) {
                        // i = index of wave node
                        for (int i = 0; i < wave.size(); ++i)
                        {
                            if (sumsOfOnes[i] == 1 && observed[i] == -1) {
                                // t = index of pattern
                                for (int t = 0; t < patternsTotal; ++t)
                                {
                                    if (wave[i][t]) {
                                        observed[i] = t;
                                        mutex->lock();
                                        Paint(outputMap, i, t);
                                        mutex->unlock();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else {
                if (!realTimePainting) {
                    // i = index of wave node
                    for (int i = 0; i < wave.size(); ++i) {
                        // t = index of pattern
                        for (int t = 0; t < patternsTotal; ++t)
                        {
                            if (wave[i][t]) {
                                observed[i] = t;
                                mutex->lock();
                                Paint(outputMap, i, t);
                                mutex->unlock();
                                break;
                            }
                        }
                    }
                }
                return true;
            }
        }
        else {
            if (node >= 0) {
                // Una vez seleccionado un nodo, guardamos datos para poder hacer backtracking a éste. Si probamos todas las posibles observaciones y no dan resultado,
                // tendríamos que hacer backtrack hacia el anterior nodo seleccionado y probar otras observaciones
                int possibilities = sumsOfOnes[node];
                calculateDist = true;

                currentCheckpoint.currentNode = node;
                currentCheckpoint.observationsMade.clear();
                currentCheckpoint.itemsStacked.clear();
                currentCheckpoint.unobserveNodes.clear();

                success = false;
                int observedPattern;
                while ((fails <= limit || limit < 0) && possibilities > 0 && !success) {
                    // Mirar qué cambia en Observe y guardarlo antes
                    observedPattern = Observe(node);
                    currentCheckpoint.observationsMade.insert(observedPattern);

                    // Mirar qué cambia Propagate y guardarlo antes
                    success = Propagate();
                    if (success) {
                        if (realTimePainting) {
                            // i = index of wave node
                            for (int i = 0; i < wave.size(); ++i)
                            {
                                if (sumsOfOnes[i] == 1 && observed[i] == -1) {
                                    // t = index of pattern
                                    for (int t = 0; t < patternsTotal; ++t)
                                    {
                                        if (wave[i][t]) {
                                            observed[i] = t;
                                            mutex->lock();
                                            Paint(outputMap, i, t);
                                            mutex->unlock();
                                            currentCheckpoint.unobserveNodes.push_back(i);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        while (limit >= 0 && limit - fails > 0 && backtrackCheckpoints.size() >= limit - fails) {
                            // Quitamos backtrackCheckpoints hasta quedarse con limit - fails - 1, para poder añadirle el current
                            BacktrackData auxCheckpoint = backtrackCheckpoints.front();
                            backtrackCheckpoints.pop_front();
                            backtrackStack.erase(backtrackStack.begin(), auxCheckpoint.lastItem);
                        }
                        if (limit - fails > 0 || limit < 0) {
                            currentCheckpoint.lastItem = prev(backtrackStack.end());
                            backtrackCheckpoints.push_back(currentCheckpoint);
                        }
                    }
                    else {
                        --possibilities;
                        if (countObservationAsFail || possibilities == 0) ++fails;
                        if (fails > limit && limit >= 0) {
                            if (!realTimePainting) {
                                // i = index of wave node
                                for (int i = 0; i < wave.size(); ++i) {
                                    // t = index of pattern
                                    for (int t = 0; t < patternsTotal; ++t)
                                    {
                                        if (wave[i][t]) {
                                            observed[i] = t;
                                            mutex->lock();
                                            Paint(outputMap, i, t);
                                            mutex->unlock();
                                            break;
                                        }
                                    }
                                }
                            }
                            return false;
                        }
                        // Recuperamos los valores del nodo actual
                        distribution[observedPattern] = 0.0;

                        list<BacktrackStackItem>::iterator firstItem = backtrackCheckpoints.empty() ? backtrackStack.begin() : next(backtrackCheckpoints.back().lastItem);
                        for (auto it = firstItem; it != backtrackStack.end(); ++it) {
                            BacktrackStackItem item = *it;
                            int nodeID = item.nodeID;
                            int patternID = item.patternID;

                            if (!wave[nodeID][patternID]) {
                                wave[nodeID][patternID] = true;

                                sumsOfOnes[nodeID] += 1;
                                sumsOfWeights[nodeID] += weights[patternID];
                                sumsOfWeightLogWeights[nodeID] += weightLogWeights[patternID];

                                double weightSum = sumsOfWeights[nodeID];
                                entropies[nodeID] = log(weightSum) - sumsOfWeightLogWeights[nodeID] / weightSum;
                            }
                            compatible[nodeID][patternID][0] = item.compDir0;
                            compatible[nodeID][patternID][1] = item.compDir1;
                            compatible[nodeID][patternID][2] = item.compDir2;
                            compatible[nodeID][patternID][3] = item.compDir3;
                        }
                        backtrackStack.erase(firstItem, backtrackStack.end());
                        currentCheckpoint.itemsStacked.clear();

                        while (possibilities == 0 && backtrackCheckpoints.size() > 0) {
                            // Recuperamos los valores del nodo anterior
                            currentCheckpoint = backtrackCheckpoints.back();
                            backtrackCheckpoints.pop_back();
                            node = currentCheckpoint.currentNode;

                            possibilities = sumsOfOnes[node] - currentCheckpoint.observationsMade.size();
                            calculateDist = true;

                            for (int i = 0; i < currentCheckpoint.unobserveNodes.size(); ++i) {
                                mutex->lock();
                                outputMap->call_deferred("erase_cell", gd::Vector2i(currentCheckpoint.unobserveNodes[i] % outputWidth, currentCheckpoint.unobserveNodes[i] / outputWidth));
                                mutex->unlock();

                                observed[currentCheckpoint.unobserveNodes[i]] = -1;
                            }
                            currentCheckpoint.unobserveNodes.clear();

                            list<BacktrackStackItem>::iterator firstItem = backtrackCheckpoints.empty() ? backtrackStack.begin() : next(backtrackCheckpoints.back().lastItem);
                            for (auto it = firstItem; it != backtrackStack.end(); ++it) {
                                BacktrackStackItem item = *it;
                                int nodeID = item.nodeID;
                                int patternID = item.patternID;

                                if (!wave[nodeID][patternID]) {
                                    wave[nodeID][patternID] = true;

                                    sumsOfOnes[nodeID] += 1;
                                    sumsOfWeights[nodeID] += weights[patternID];
                                    sumsOfWeightLogWeights[nodeID] += weightLogWeights[patternID];

                                    double weightSum = sumsOfWeights[nodeID];
                                    entropies[nodeID] = log(weightSum) - sumsOfWeightLogWeights[nodeID] / weightSum;
                                }
                                compatible[nodeID][patternID][0] = item.compDir0;
                                compatible[nodeID][patternID][1] = item.compDir1;
                                compatible[nodeID][patternID][2] = item.compDir2;
                                compatible[nodeID][patternID][3] = item.compDir3;
                            }
                            backtrackStack.erase(firstItem, backtrackStack.end());
                            currentCheckpoint.itemsStacked.clear();
                        }
                        if (possibilities == 0 && backtrackCheckpoints.size() == 0) {
                            return false;
                        }
                    }
                }
            }
            else {
                if (!realTimePainting) {
                    // i = index of wave node
                    for (int i = 0; i < wave.size(); ++i) {
                        // t = index of pattern
                        for (int t = 0; t < patternsTotal; ++t)
                        {
                            if (wave[i][t]) {
                                observed[i] = t;
                                mutex->lock();
                                Paint(outputMap, i, t);
                                mutex->unlock();
                                break;
                            }
                        }
                    }
                }
                return true;
            }
        }
    }
    if (!realTimePainting) {
        // i = index of wave node
        for (int i = 0; i < wave.size(); ++i) {
            // t = index of pattern
            for (int t = 0; t < patternsTotal; ++t)
            {
                if (wave[i][t]) {
                    observed[i] = t;
                    mutex->lock();
                    Paint(outputMap, i, t);
                    mutex->unlock();
                    break;
                }
            }
        }
    }
    return true;
}

/*
    Chooses the next node to observe based on certain heuristics
*/
int Model::NextUnobservedNode() {
    double min = 1E+4;
    int argmin = -1;
    // i = index of wave node
    for (int i = 0; i < wave.size(); ++i) {
        int remainingValues = sumsOfOnes[i];
        double entropy = (heuristic == Heuristic::Entropy) ? entropies[i] : remainingValues;
        if (remainingValues > 1 && entropy <= min) {
            double noise = 1E-6 * rng->randf();
            if (entropy + noise < min) {
                min = entropy + noise;
                argmin = i;
            }
        }
    }
    return argmin;
}

/*
    Observes the node given
*/
int Model::Observe(int node) {
    vector<bool>& nodeWave = wave[node];

    if (calculateDist) {
        // t = index of pattern
        for (int t = 0; t < patternsTotal; ++t)
            distribution[t] = (nodeWave[t] && (currentCheckpoint.observationsMade.count(t) == 0)) ? weights[t] : 0.0;
        calculateDist = !backtrack;
    }
    
    int r = RandomDistProb(distribution, rng->randf());
    // t = index of pattern
    for (int t = 0; t < patternsTotal; ++t)
        if (nodeWave[t] != (t == r)) Ban(node, t);
    
    return r;
}

/*
    Propagates the local restrictions after an observation
*/
bool Model::Propagate() {
    while (stackSize > 0) {
        pair<int, int> currentPair = stack[--stackSize];
        int currentNode = currentPair.first;
        int currentPattern = currentPair.second;

        int currentNodeX = currentNode % outputWidth;
        int currentNodeY = currentNode / outputWidth;

        // d = direction
        for (int d = 0; d < 4; ++d) {
            int nextNodeX = currentNodeX + directionX[d];
            int nextNodeY = currentNodeY + directionY[d];
            if (!periodic && (nextNodeX < 0 || nextNodeY < 0 || nextNodeX >= outputWidth || nextNodeY >= outputHeight)) continue;

            if (nextNodeX < 0) nextNodeX += outputWidth;
            else if (nextNodeX >= outputWidth) nextNodeX -= outputWidth;
            if (nextNodeY < 0) nextNodeY += outputHeight;
            else if (nextNodeY >= outputHeight) nextNodeY -= outputHeight;

            int nextNode = nextNodeX + nextNodeY * outputWidth;
            const vector<int>& prop = propagator[d][currentPattern];
            vector<vector<int>>& compat = compatible[nextNode];

            // l = index of compatible pattern in direction d
            for (int l = 0; l < prop.size(); ++l) {
                int nextPattern = prop[l];
                vector<int>& comp = compat[nextPattern];

                if (comp[d] == 1) Ban(nextNode, nextPattern);
                else if (comp[d] > 0) {
                    pair<int, int> IDs(nextNode, nextPattern);
                    if (backtrack && (currentCheckpoint.itemsStacked.count(IDs) == 0)) {
                        BacktrackStackItem item;
                        item.nodeID = nextNode;
                        item.patternID = nextPattern;

                        item.compDir0 = comp[0];
                        item.compDir1 = comp[1];
                        item.compDir2 = comp[2];
                        item.compDir3 = comp[3];

                        backtrackStack.push_back(item);
                        currentCheckpoint.itemsStacked.insert(IDs);
                    }
                    
                    comp[d]--;
                }
            }
        }
    }

    return sumsOfOnes[0] > 0;
}

// Marca un patrón t como imposible en una determinada celda i
void Model::Ban(int i, int t) {
    wave[i][t] = false;
    vector<int>& comp = compatible[i][t];

    pair<int, int> IDs(i, t);
    if (backtrack && (currentCheckpoint.itemsStacked.count(IDs) == 0)) {
        BacktrackStackItem item;
        item.nodeID = i;
        item.patternID = t;

        item.compDir0 = comp[0];
        item.compDir1 = comp[1];
        item.compDir2 = comp[2];
        item.compDir3 = comp[3];

        backtrackStack.push_back(item);
        currentCheckpoint.itemsStacked.insert(IDs);
    }

    // d = direction
    for (int d = 0; d < 4; ++d) comp[d] = 0;
    stack[stackSize++] = { i, t };

    sumsOfOnes[i] -= 1;
    sumsOfWeights[i] -= weights[t];
    sumsOfWeightLogWeights[i] -= weightLogWeights[t];

    double weightSum = sumsOfWeights[i];
    entropies[i] = log(weightSum) - sumsOfWeightLogWeights[i] / weightSum;
}

// Marca todas las celdas como posibles para todos los patrones y restablece los contadores
void Model::Clear(gd::Ref<gd::Mutex> mutex, gd::TileMapLayer* outputMap) {
    // i = index of wave node
    for (int i = 0; i < wave.size(); ++i) {
        if(notFilledWave) fill(wave[i].begin(), wave[i].end(), true);
        notFilledWave = true;

        // t = index of pattern
        for (int t = 0; t < patternsTotal; ++t) {
            // d = direction
            for (int d = 0; d < 4; ++d) {
                compatible[i][t][d] = propagator[opposite[d]][t].size();
            }
                
        }
        sumsOfOnes[i] = weights.size();
        sumsOfWeights[i] = sumOfWeights;
        sumsOfWeightLogWeights[i] = sumOfWeightLogWeights;
        entropies[i] = startingEntropy;
        observed[i] = -1;
    }
    observedSoFar = 0;
    calculateDist = true;
    backtrack = false;

    backtrackStack.clear();
    backtrackCheckpoints.clear();
    mutex->lock();
    outputMap->call_deferred("clear");
    mutex->unlock();

    if (ground) {
        vector<bool> areGroundPatterns(patternsTotal, false);

        for (int g = 0; g < groundPatterns.size(); ++g) {
            areGroundPatterns[groundPatterns[g]] = true;
            for (int x = 0; x < outputWidth; ++x) {
                for (int y = 0; y < outputHeight - 1; ++y) Ban(x + y * outputWidth, groundPatterns[g]);
            }
        }

        // t = index of pattern
        for (int t = 0; t < patternsTotal; ++t) {
            if (areGroundPatterns[t]) continue;
            for (int x = 0; x < outputWidth; ++x) {
                Ban(x + (outputHeight - 1) * outputWidth, t);
            }
        }
    }
}

// Función para obtener un valor basado en pesos (equivalente a Random en C#)
int Model::RandomDistProb(const vector<double>& weights, double rand) {
    double weightSum = 0;
    for (double weight : weights) weightSum += weight;
    double threshold = rand * weightSum;

    double partialSum = 0;
    for (int i = 0; i < weights.size(); i++) {
        partialSum += weights[i];
        if (partialSum >= threshold) return i;
    }
    return 0;
}