
#include "ids_engine.h"
#include "movegen.h"
#include "evaluator.h"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <iomanip>

using namespace std;


static int idsNodeCounter = 0;


static int minimaxIDS(Board& board, int depth, int alpha, int beta, bool isMaximizing) {
    idsNodeCounter++;


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
            int score = minimaxIDS(board, depth - 1, alpha, beta, false);
            board.unmakeMove(move);
            if (score > bestScore) bestScore = score;
            if (score > alpha) alpha = score;
            if (beta <= alpha) break; 
        }
        return bestScore;
    } else {

        int bestScore = INF_SCORE;
        for (const Move& move : moves) {
            board.makeMove(move);
            int score = minimaxIDS(board, depth - 1, alpha, beta, true);
            board.unmakeMove(move);
            if (score < bestScore) bestScore = score;
            if (score < beta) beta = score;
            if (beta <= alpha) break;  
        }
        return bestScore;
    }
}


IDSResult findBestMoveIDS(Board& board, int maxDepth) {
    IDSResult result;
    result.totalNodes = 0;
    result.totalTimeMs = 0;
    result.score = 0;
    result.bestMove = Move();

    auto totalStart = chrono::high_resolution_clock::now();


    vector<Move> moves = generateLegalMoves(board);
    if (moves.empty()) {
        return result;
    }

    bool isMaximizing = (board.sideToMove == WHITE);
    Move bestMoveSoFar = moves[0];

    // Print IDS header
    cout << endl;
    cout << "  +=========================================================+" << endl;
    cout << "  |     ITERATIVE DEEPENING SEARCH (IDS)                    |" << endl;
    cout << "  |     Move Ordering Optimization Demo                     |" << endl;
    cout << "  +---------+-----------+-----------+-----------+-----------+" << endl;
    cout << "  |  Depth  |   Nodes   |  Time(ms) | Best Move |   Score   |" << endl;
    cout << "  +---------+-----------+-----------+-----------+-----------+" << endl;


    for (int depth = 1; depth <= maxDepth; depth++) {
        auto iterStart = chrono::high_resolution_clock::now();
        idsNodeCounter = 0;


        orderMoves(board, moves);


        if (depth > 1) {
            for (int i = 0; i < (int)moves.size(); i++) {
                if (moves[i] == bestMoveSoFar) {

                    swap(moves[0], moves[i]);
                    break;
                }
            }
        }


        int bestScore = isMaximizing ? -INF_SCORE : INF_SCORE;
        Move bestMove = moves[0];
        int alpha = -INF_SCORE;
        int beta  =  INF_SCORE;

        for (const Move& move : moves) {
            board.makeMove(move);
            int score = minimaxIDS(board, depth - 1, alpha, beta, !isMaximizing);
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

        auto iterEnd = chrono::high_resolution_clock::now();
        double iterTime = (double)chrono::duration_cast<chrono::milliseconds>(
            iterEnd - iterStart).count();


        IDSIterationStats stats;
        stats.depth = depth;
        stats.nodesSearched = idsNodeCounter;
        stats.bestMove = bestMove;
        stats.score = bestScore;
        stats.timeMs = iterTime;
        result.iterations.push_back(stats);

        result.totalNodes += idsNodeCounter;
        bestMoveSoFar = bestMove;

 
        cout << "  |  " << setw(5) << left << depth << right
             << "  | " << setw(9) << idsNodeCounter
             << " | " << setw(9) << fixed << setprecision(0) << iterTime
             << " | " << setw(9) << left << bestMove.toString() << right
             << " | " << setw(9) << fixed << setprecision(1) << (bestScore / 100.0)
             << " |" << endl;
    }

    auto totalEnd = chrono::high_resolution_clock::now();
    result.totalTimeMs = (double)chrono::duration_cast<chrono::milliseconds>(
        totalEnd - totalStart).count();


    cout << "  +---------+-----------+-----------+-----------+-----------+" << endl;
    cout << "  | Total nodes searched: " << setw(10) << left << result.totalNodes << right
         << "                            |" << endl;
    cout << "  | Total time:           " << setw(8) << fixed << setprecision(0)
         << result.totalTimeMs << " ms"
         << "                          |" << endl;
    cout << "  +---------+-----------+-----------+-----------+-----------+" << endl;


    if (result.iterations.size() >= 2) {
        cout << endl;
        cout << "  --- Move Ordering Efficiency ---" << endl;
        cout << "  IDS reuses the best move from depth (D-1) to order" << endl;
        cout << "  moves at depth D. Combined with MVV-LVA capture" << endl;
        cout << "  ordering, this tightens alpha-beta bounds early," << endl;
        cout << "  pruning large portions of the game tree." << endl;
        cout << endl;


        for (int i = 1; i < (int)result.iterations.size(); i++) {
            double ratio = (result.iterations[i - 1].nodesSearched > 0)
                ? (double)result.iterations[i].nodesSearched / result.iterations[i - 1].nodesSearched
                : 0;
            cout << "  Depth " << result.iterations[i - 1].depth
                 << " -> " << result.iterations[i].depth
                 << ":  branching factor ~" << fixed << setprecision(1) << ratio
                 << "x  (ideal ≈ sqrt(b))" << endl;
        }
    }

    result.bestMove = bestMoveSoFar;
    result.score = result.iterations.back().score;

    return result;
}
