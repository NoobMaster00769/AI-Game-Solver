
#include "ai_engine.h"
#include "movegen.h"
#include "evaluator.h"
#include <chrono>
#include <iostream>
#include <algorithm>

using namespace std;


static int nodeCounter = 0;


static int minimax(Board& board, int depth, int alpha, int beta, bool isMaximizing) {

    nodeCounter++;


    if (depth == 0) {
        return evaluate(board);
    }


    vector<Move> moves = generateLegalMoves(board);

    if (moves.empty()) {
        if (board.isInCheck(board.sideToMove)) {

            if (isMaximizing) {
                return -MATE_SCORE + (100 - depth);  
            } else {
                return MATE_SCORE - (100 - depth);   
            }
        }
        return 0;  
    }


    if (board.isDraw()) {
        return 0;
    }


    orderMoves(board, moves);

    if (isMaximizing) {

        int bestScore = -INF_SCORE;

        for (const Move& move : moves) {

            board.makeMove(move);


            int score = minimax(board, depth - 1, alpha, beta, false);


            board.unmakeMove(move);

   
            if (score > bestScore) {
                bestScore = score;
            }


            if (score > alpha) {
                alpha = score;
            }

 
            if (beta <= alpha) {
                break;  // Prune!
            }
        }

        return bestScore;

    } else {

        int bestScore = INF_SCORE;

        for (const Move& move : moves) {

            board.makeMove(move);


            int score = minimax(board, depth - 1, alpha, beta, true);


            board.unmakeMove(move);


            if (score < bestScore) {
                bestScore = score;
            }


            if (score < beta) {
                beta = score;
            }


            if (beta <= alpha) {
                break;  
            }
        }

        return bestScore;
    }
}


SearchResult findBestMove(Board& board, int depth) {
    SearchResult result;
    result.bestMove = Move();  
    result.score = 0;
    result.nodesSearched = 0;
    result.timeMs = 0;


    auto startTime = chrono::high_resolution_clock::now();


    nodeCounter = 0;

    vector<Move> moves = generateLegalMoves(board);

    if (moves.empty()) {

        return result;
    }


    orderMoves(board, moves);


    bool isMaximizing = (board.sideToMove == WHITE);

    int bestScore = isMaximizing ? -INF_SCORE : INF_SCORE;
    Move bestMove = moves[0];  

    int alpha = -INF_SCORE;
    int beta  =  INF_SCORE;


    for (const Move& move : moves) {
        board.makeMove(move);


        int score = minimax(board, depth - 1, alpha, beta, !isMaximizing);

        board.unmakeMove(move);


        if (isMaximizing) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
            if (score > alpha) alpha = score;
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = move;
            }
            if (score < beta) beta = score;
        }
    }


    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);


    result.bestMove = bestMove;
    result.score = bestScore;
    result.nodesSearched = nodeCounter;
    result.timeMs = (double)duration.count();

    return result;
}


static bool compareMaximizing(const EvaluatedMove& a, const EvaluatedMove& b) {
    return a.score > b.score;
}

static bool compareMinimizing(const EvaluatedMove& a, const EvaluatedMove& b) {
    return a.score < b.score;
}

vector<EvaluatedMove> findTopMoves(Board& board, int depth, int count) {
    vector<EvaluatedMove> topMoves;

    vector<Move> moves = generateLegalMoves(board);
    if (moves.empty()) {
        return topMoves;
    }

    orderMoves(board, moves);

    bool isMaximizing = (board.sideToMove == WHITE);

    int alpha = -INF_SCORE;
    int beta  =  INF_SCORE;

    for (const Move& move : moves) {
        board.makeMove(move);
        int score = minimax(board, depth - 1, alpha, beta, !isMaximizing);
        board.unmakeMove(move);

        topMoves.push_back({move, score});


    }

    if (isMaximizing) {
        sort(topMoves.begin(), topMoves.end(), compareMaximizing);
    } else {
        sort(topMoves.begin(), topMoves.end(), compareMinimizing);
    }

    if (topMoves.size() > (size_t)count) {
        topMoves.resize(count);
    }

    return topMoves;
}
