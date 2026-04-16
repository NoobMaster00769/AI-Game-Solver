
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "board.h"
#include <vector>

using namespace std;


vector<Move> generateLegalMoves(Board& board);


void orderMoves(const Board& board, vector<Move>& moves);

#endif 
