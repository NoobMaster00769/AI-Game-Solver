
#include "movegen.h"
#include <algorithm>

using namespace std;


static void addSlidingMoves(const Board& board, int fromSq, 
                            int dirRank, int dirFile,
                            Color color, vector<Move>& moves) {
    int rank = squareRank(fromSq);
    int file = squareFile(fromSq);

    for (int dist = 1; dist < 8; dist++) {
        int r = rank + dirRank * dist;
        int f = file + dirFile * dist;

        if (!isValidRankFile(r, f)) break; 

        int toSq = makeSquare(r, f);
        Piece target = board.board[toSq];

        if (target == NO_PIECE) {

            moves.push_back(Move(fromSq, toSq));
        } else if (pieceColor(target) != color) {

            moves.push_back(Move(fromSq, toSq));
            break;
        } else {

            break;
        }
    }
}


static vector<Move> generatePseudoLegalMoves(const Board& board) {
    vector<Move> moves;
    moves.reserve(128);  

    Color color = board.sideToMove;


    for (int sq = 0; sq < 64; sq++) {
        Piece piece = board.board[sq];


        if (piece == NO_PIECE) continue;
        if (pieceColor(piece) != color) continue;

        PieceType pt = pieceType(piece);
        int rank = squareRank(sq);
        int file = squareFile(sq);


        if (pt == PAWN) {
            int direction = (color == WHITE) ? 1 : -1;   
            int startRank = (color == WHITE) ? 1 : 6;     
            int promoRank = (color == WHITE) ? 7 : 0;     


            int pushRank = rank + direction;
            if (isValidRankFile(pushRank, file)) {
                int pushSq = makeSquare(pushRank, file);
                if (board.board[pushSq] == NO_PIECE) {
                    if (pushRank == promoRank) {

                        moves.push_back(Move(sq, pushSq, FLAG_PROMOTION));
                    } else {
                        moves.push_back(Move(sq, pushSq));
                    }

                    if (rank == startRank) {
                        int doublePushRank = rank + 2 * direction;
                        int doublePushSq = makeSquare(doublePushRank, file);
                        if (board.board[doublePushSq] == NO_PIECE) {
                            moves.push_back(Move(sq, doublePushSq));
                        }
                    }
                }
            }


            for (int df = -1; df <= 1; df += 2) {  
                int capFile = file + df;
                int capRank = rank + direction;
                if (!isValidRankFile(capRank, capFile)) continue;

                int capSq = makeSquare(capRank, capFile);


                if (board.board[capSq] != NO_PIECE && pieceColor(board.board[capSq]) != color) {
                    if (capRank == promoRank) {
                        moves.push_back(Move(sq, capSq, FLAG_PROMOTION));
                    } else {
                        moves.push_back(Move(sq, capSq));
                    }
                }


                if (capSq == board.enPassantSquare) {
                    moves.push_back(Move(sq, capSq, FLAG_EN_PASSANT));
                }
            }
        }


        else if (pt == KNIGHT) {
            int offsets[8][2] = {
                {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                { 1, -2}, { 1, 2}, { 2, -1}, { 2, 1}
            };
            for (int i = 0; i < 8; i++) {
                int r = rank + offsets[i][0];
                int f = file + offsets[i][1];
                if (!isValidRankFile(r, f)) continue;
                int toSq = makeSquare(r, f);
                Piece target = board.board[toSq];
                // Can move if square is empty or has an enemy piece
                if (target == NO_PIECE || pieceColor(target) != color) {
                    moves.push_back(Move(sq, toSq));
                }
            }
        }


        else if (pt == BISHOP) {
            addSlidingMoves(board, sq,  1,  1, color, moves);  
            addSlidingMoves(board, sq,  1, -1, color, moves);  
            addSlidingMoves(board, sq, -1,  1, color, moves);  
            addSlidingMoves(board, sq, -1, -1, color, moves);  
        }


        else if (pt == ROOK) {
            addSlidingMoves(board, sq,  1,  0, color, moves);  
            addSlidingMoves(board, sq, -1,  0, color, moves);  
            addSlidingMoves(board, sq,  0,  1, color, moves);  
            addSlidingMoves(board, sq,  0, -1, color, moves);  
        }

  
        else if (pt == QUEEN) {

            addSlidingMoves(board, sq,  1,  1, color, moves);
            addSlidingMoves(board, sq,  1, -1, color, moves);
            addSlidingMoves(board, sq, -1,  1, color, moves);
            addSlidingMoves(board, sq, -1, -1, color, moves);

            addSlidingMoves(board, sq,  1,  0, color, moves);
            addSlidingMoves(board, sq, -1,  0, color, moves);
            addSlidingMoves(board, sq,  0,  1, color, moves);
            addSlidingMoves(board, sq,  0, -1, color, moves);
        }


        else if (pt == KING) {

            for (int dr = -1; dr <= 1; dr++) {
                for (int df = -1; df <= 1; df++) {
                    if (dr == 0 && df == 0) continue;
                    int r = rank + dr;
                    int f = file + df;
                    if (!isValidRankFile(r, f)) continue;
                    int toSq = makeSquare(r, f);
                    Piece target = board.board[toSq];
                    if (target == NO_PIECE || pieceColor(target) != color) {
                        moves.push_back(Move(sq, toSq));
                    }
                }
            }


            Color enemy = oppositeColor(color);

            if (!board.isInCheck(color)) {

                bool canCastleKing = (color == WHITE) ? board.castleWhiteKing : board.castleBlackKing;
                if (canCastleKing) {
                    int f_sq = makeSquare(rank, 5);  
                    int g_sq = makeSquare(rank, 6);  

                    if (board.board[f_sq] == NO_PIECE && board.board[g_sq] == NO_PIECE) {

                        if (!board.isSquareAttacked(f_sq, enemy) &&
                            !board.isSquareAttacked(g_sq, enemy)) {
                            moves.push_back(Move(sq, g_sq, FLAG_CASTLE));
                        }
                    }
                }


                bool canCastleQueen = (color == WHITE) ? board.castleWhiteQueen : board.castleBlackQueen;
                if (canCastleQueen) {
                    int d_sq = makeSquare(rank, 3);  
                    int c_sq = makeSquare(rank, 2);  
                    int b_sq = makeSquare(rank, 1);  

                    if (board.board[d_sq] == NO_PIECE &&
                        board.board[c_sq] == NO_PIECE &&
                        board.board[b_sq] == NO_PIECE) {

                        if (!board.isSquareAttacked(d_sq, enemy) &&
                            !board.isSquareAttacked(c_sq, enemy)) {
                            moves.push_back(Move(sq, c_sq, FLAG_CASTLE));
                        }
                    }
                }
            }
        }
    }

    return moves;
}


vector<Move> generateLegalMoves(Board& board) {
    vector<Move> pseudoMoves = generatePseudoLegalMoves(board);
    vector<Move> legalMoves;
    legalMoves.reserve(pseudoMoves.size());

    for (const Move& move : pseudoMoves) {
        // Try the move
        board.makeMove(move);


        if (!board.isInCheck(oppositeColor(board.sideToMove))) {
            legalMoves.push_back(move);  
        }


        board.unmakeMove(move);
    }

    return legalMoves;
}




static const int PIECE_ORDER_VALUE[] = {
    100,   
    320,   
    330,   
    500,   
    900,   
    20000  
};

void orderMoves(const Board& board, vector<Move>& moves) {

    sort(moves.begin(), moves.end(),
        [&board](const Move& a, const Move& b) {
            int scoreA = 0, scoreB = 0;


            if (board.board[a.to] != NO_PIECE) {
                scoreA = PIECE_ORDER_VALUE[pieceType(board.board[a.to])] * 10
                       - PIECE_ORDER_VALUE[pieceType(board.board[a.from])];
            }
            if (board.board[b.to] != NO_PIECE) {
                scoreB = PIECE_ORDER_VALUE[pieceType(board.board[b.to])] * 10
                       - PIECE_ORDER_VALUE[pieceType(board.board[b.from])];
            }


            if (a.flag == FLAG_PROMOTION) scoreA += 8000;
            if (b.flag == FLAG_PROMOTION) scoreB += 8000;


            if (a.flag == FLAG_EN_PASSANT) scoreA += 500;
            if (b.flag == FLAG_EN_PASSANT) scoreB += 500;

            return scoreA > scoreB;  
        }
    );
}
