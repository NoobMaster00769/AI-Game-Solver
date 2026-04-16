
#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "board.h"
#include "types.h"
#include <vector>

using namespace std;

struct SearchResult {
    Move   bestMove;       
    int    score;          
    int    nodesSearched;  
    double timeMs;         
};


SearchResult findBestMove(Board& board, int depth);


struct EvaluatedMove {
    Move move;
    int score;
};


vector<EvaluatedMove> findTopMoves(Board& board, int depth, int count = 3);

#endif 
