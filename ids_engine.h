
#ifndef IDS_ENGINE_H
#define IDS_ENGINE_H

#include "board.h"
#include "types.h"
#include "ai_engine.h"
#include <vector>
#include <string>

using namespace std;


struct IDSIterationStats {
    int depth;           
    int nodesSearched;   
    Move bestMove;       
    int score;           
    double timeMs;       
};


struct IDSResult {
    Move bestMove;       
    int score;           
    int totalNodes;      
    double totalTimeMs;  
    vector<IDSIterationStats> iterations;  
};


IDSResult findBestMoveIDS(Board& board, int maxDepth);

#endif 