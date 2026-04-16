
#include "evaluator.h"
#include "movegen.h"

using namespace std;


static const int PIECE_VALUES[] = {
    100,    
    320,    
    330,    
    500,    
    900,    
    20000   
};




static const int PAWN_TABLE[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,   
     5, 10, 10,-20,-20, 10, 10,  5,   
     5, -5,-10,  0,  0,-10, -5,  5,   
     0,  0,  0, 20, 20,  0,  0,  0,   
     5,  5, 10, 25, 25, 10,  5,  5,   
    10, 10, 20, 30, 30, 20, 10, 10,   
    50, 50, 50, 50, 50, 50, 50, 50,   
     0,  0,  0,  0,  0,  0,  0,  0    
};


static const int KNIGHT_TABLE[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,   
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,   
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};


static const int BISHOP_TABLE[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};


static const int ROOK_TABLE[64] = {
      0,  0,  0,  5,  5,  0,  0,  0,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      5, 10, 10, 10, 10, 10, 10,  5,   
      0,  0,  0,  0,  0,  0,  0,  0
};


static const int QUEEN_TABLE[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -10,  5,  5,  5,  5,  5,  0,-10,
      0,  0,  5,  5,  5,  5,  0, -5,
     -5,  0,  5,  5,  5,  5,  0, -5,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};


static const int KING_TABLE_MIDDLEGAME[64] = {
     20, 30, 10,  0,  0, 10, 30, 20,   
     20, 20,  0,  0,  0,  0, 20, 20,
    -10,-20,-20,-20,-20,-20,-20,-10,   
    -20,-30,-30,-40,-40,-30,-30,-20,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30
};


static const int KING_TABLE_ENDGAME[64] = {
    -50,-30,-30,-30,-30,-30,-30,-50,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,   
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -50,-40,-30,-20,-20,-30,-40,-50
};


static int getPieceSquareValue(PieceType pt, Color color, int square, bool isEndgame) {

    int lookupSquare = (color == WHITE) ? square : (56 + squareFile(square) - 8 * squareRank(square));

    if (color == BLACK) {
        int rank = squareRank(square);
        int file = squareFile(square);
        lookupSquare = makeSquare(7 - rank, file);
    }

    switch (pt) {
        case PAWN:   return PAWN_TABLE[lookupSquare];
        case KNIGHT: return KNIGHT_TABLE[lookupSquare];
        case BISHOP: return BISHOP_TABLE[lookupSquare];
        case ROOK:   return ROOK_TABLE[lookupSquare];
        case QUEEN:  return QUEEN_TABLE[lookupSquare];
        case KING:
            if (isEndgame) return KING_TABLE_ENDGAME[lookupSquare];
            else           return KING_TABLE_MIDDLEGAME[lookupSquare];
        default:     return 0;
    }
}


int evaluate(const Board& board) {
    int whiteScore = 0;
    int blackScore = 0;


    int totalMaterial = 0;
    for (int sq = 0; sq < 64; sq++) {
        Piece p = board.board[sq];
        if (p == NO_PIECE) continue;
        PieceType pt = pieceType(p);
        if (pt != PAWN && pt != KING) {
            totalMaterial += PIECE_VALUES[pt];
        }
    }

    bool isEndgame = (totalMaterial < 2600);  


    for (int sq = 0; sq < 64; sq++) {
        Piece p = board.board[sq];
        if (p == NO_PIECE) continue;

        PieceType pt = pieceType(p);
        Color color = pieceColor(p);


        int value = PIECE_VALUES[pt];


        value += getPieceSquareValue(pt, color, sq, isEndgame);


        if (color == WHITE) {
            whiteScore += value;
        } else {
            blackScore += value;
        }
    }


    for (int file = 0; file < 8; file++) {
        int whitePawnsOnFile = 0;
        int blackPawnsOnFile = 0;

        for (int rank = 0; rank < 8; rank++) {
            Piece p = board.board[makeSquare(rank, file)];
            if (p == W_PAWN) whitePawnsOnFile++;
            if (p == B_PAWN) blackPawnsOnFile++;
        }


        if (whitePawnsOnFile > 1) whiteScore -= 15 * (whitePawnsOnFile - 1);
        if (blackPawnsOnFile > 1) blackScore -= 15 * (blackPawnsOnFile - 1);


        if (whitePawnsOnFile > 0) {
            bool hasNeighbor = false;
            if (file > 0) {
                for (int r = 0; r < 8; r++) {
                    if (board.board[makeSquare(r, file - 1)] == W_PAWN) { hasNeighbor = true; break; }
                }
            }
            if (!hasNeighbor && file < 7) {
                for (int r = 0; r < 8; r++) {
                    if (board.board[makeSquare(r, file + 1)] == W_PAWN) { hasNeighbor = true; break; }
                }
            }
            if (!hasNeighbor) whiteScore -= 10;
        }

        if (blackPawnsOnFile > 0) {
            bool hasNeighbor = false;
            if (file > 0) {
                for (int r = 0; r < 8; r++) {
                    if (board.board[makeSquare(r, file - 1)] == B_PAWN) { hasNeighbor = true; break; }
                }
            }
            if (!hasNeighbor && file < 7) {
                for (int r = 0; r < 8; r++) {
                    if (board.board[makeSquare(r, file + 1)] == B_PAWN) { hasNeighbor = true; break; }
                }
            }
            if (!hasNeighbor) blackScore -= 10;
        }
    }


    int whiteBishops = 0, blackBishops = 0;
    for (int sq = 0; sq < 64; sq++) {
        if (board.board[sq] == W_BISHOP) whiteBishops++;
        if (board.board[sq] == B_BISHOP) blackBishops++;
    }
    if (whiteBishops >= 2) whiteScore += 30;
    if (blackBishops >= 2) blackScore += 30;


    return whiteScore - blackScore;
}
