
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "types.h"
#include "board.h"
#include "movegen.h"
#include "evaluator.h"
#include "ai_engine.h"
#include "ids_engine.h"
#include "maia_engine.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

inline void runProtocolMode() {
    Board board;
    string line;

    MaiaEngine maia;
    bool maiaAttempted = false;
    bool maiaAvailable = false;

    auto ensureMaia = [&]() {
        if (!maiaAttempted) {
            maiaAttempted = true;
            string lc0Path = findLc0Path();
            if (!lc0Path.empty()) {
                string wPath = "maia-1200.pb.gz";
                FILE* f = fopen(wPath.c_str(), "rb");
                if (!f) {
                    wPath = "../maia-1200.pb.gz";
                    f = fopen(wPath.c_str(), "rb");
                }
                if (f) {
                    fclose(f);
                    maiaAvailable = maia.start(lc0Path, wPath);
                }
            }
        }
    };


    cout << "ready" << endl;

    while (getline(cin, line)) {

        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' '))
            line.pop_back();
        while (!line.empty() && line.front() == ' ')
            line.erase(line.begin());

        if (line.empty()) continue;


        string cmd = line;
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "new") {

            board = Board();
            cout << "ok" << endl;

        } else if (cmd.substr(0, 5) == "move ") {

            string moveStr = cmd.substr(5);

            while (!moveStr.empty() && moveStr.back() == ' ') moveStr.pop_back();
            while (!moveStr.empty() && moveStr.front() == ' ') moveStr.erase(moveStr.begin());

            vector<Move> legalMoves = generateLegalMoves(board);


            bool found = false;
            for (const Move& m : legalMoves) {
                if (m.toString() == moveStr) {
                    board.makeMove(m);
                    found = true;
                    break;
                }
            }

            if (found) {

                vector<Move> nextMoves = generateLegalMoves(board);
                if (nextMoves.empty()) {
                    if (board.isInCheck(board.sideToMove))
                        cout << "ok checkmate" << endl;
                    else
                        cout << "ok stalemate" << endl;
                } else if (board.isDraw()) {
                    cout << "ok draw" << endl;
                } else if (board.isInCheck(board.sideToMove)) {
                    cout << "ok check" << endl;
                } else {
                    cout << "ok" << endl;
                }
            } else {
                cout << "error invalid_move" << endl;
            }

        } else if (cmd.substr(0, 2) == "go") {

            string engineType = "minimax";
            int depth = 3;
            
            istringstream iss(cmd);
            string token;
            iss >> token; // "go"
            if (iss >> token) {
                if (token == "maia") {
                    engineType = "maia";
                } else if (token == "minimax") {
                    if (iss >> token) {
                        try { depth = stoi(token); } catch (...) {}
                    }
                } else {
                    try { depth = stoi(token); } catch (...) {}
                }
            }
            if (depth < 1) depth = 1;
            if (depth > 6) depth = 6;

            Move bestMove;
            int score = 0;
            int nodes = 0;
            double timeMs = 0;

            if (engineType == "maia") {
                ensureMaia();
                if (maiaAvailable) {
                    bestMove = maia.getBestMoveForBoard(board);
                }
            } else {
                IDSResult result = findBestMoveIDS(board, depth);
                bestMove = result.bestMove;
                score = result.score;
                nodes = result.totalNodes;
                timeMs = result.totalTimeMs;
            }

            if (bestMove.isValid()) {
                board.makeMove(bestMove);


                vector<Move> nextMoves = generateLegalMoves(board);
                string state = "";
                if (nextMoves.empty()) {
                    if (board.isInCheck(board.sideToMove))
                        state = " checkmate";
                    else
                        state = " stalemate";
                } else if (board.isDraw()) {
                    state = " draw";
                } else if (board.isInCheck(board.sideToMove)) {
                    state = " check";
                }

                cout << "bestmove " << bestMove.toString()
                     << " score " << score
                     << " nodes " << nodes
                     << " time " << (int)timeMs
                     << state << endl;
            } else {
                cout << "error no_move" << endl;
            }

        } else if (cmd == "legal") {

            vector<Move> moves = generateLegalMoves(board);
            cout << "moves";
            for (const Move& m : moves) {
                cout << " " << m.toString();
            }
            cout << endl;

        } else if (cmd == "board") {

            for (int rank = 7; rank >= 0; rank--) {
                cout << "row";
                for (int file = 0; file < 8; file++) {
                    int sq = makeSquare(rank, file);
                    Piece p = board.board[sq];
                    cout << " " << PIECE_LETTERS[p];
                }
                cout << endl;
            }
            cout << "endboard " << (board.sideToMove == WHITE ? "w" : "b") << endl;

        } else if (cmd.substr(0, 5) == "hints") {

            int depth = 3;
            if (cmd.length() > 6) {
                try {
                    depth = stoi(cmd.substr(6));
                } catch (...) {
                    depth = 3;
                }
                if (depth < 1) depth = 1;
                if (depth > 6) depth = 6;
            }

            vector<EvaluatedMove> topMoves = findTopMoves(board, depth, 3);
            for (int i = 0; i < (int)topMoves.size(); i++) {
                cout << "hint minimax " << topMoves[i].move.toString()
                     << " " << topMoves[i].score << endl;
            }

            ensureMaia();
            if (maiaAvailable) {
                Move maiaMove = maia.getBestMoveForBoard(board);
                if (maiaMove.isValid()) {
                    cout << "hint maia " << maiaMove.toString() << " 0" << endl;
                }
            }
            cout << "endhints" << endl;

        } else if (cmd == "eval") {
            int score = evaluate(board);
            cout << "eval " << score << endl;

        } else if (cmd == "side") {
            cout << "side " << (board.sideToMove == WHITE ? "w" : "b") << endl;

        } else if (cmd == "gameover") {
            vector<Move> moves = generateLegalMoves(board);
            if (moves.empty()) {
                if (board.isInCheck(board.sideToMove))
                    cout << "gameover checkmate" << endl;
                else
                    cout << "gameover stalemate" << endl;
            } else if (board.isDraw()) {
                cout << "gameover draw" << endl;
            } else {
                cout << "gameover no" << endl;
            }

        } else if (cmd == "quit") {
            cout << "bye" << endl;
            break;

        } else {
            cout << "error unknown_command" << endl;
        }
    }
}

#endif 
