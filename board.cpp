
#include "board.h"
#include <iostream>
#include <sstream>

using namespace std;


Board::Board() {
    initFromFEN(START_FEN);
}


void Board::initFromFEN(const string& fen) {

    for (int i = 0; i < 64; i++) {
        board[i] = NO_PIECE;
    }

    // Reset all state
    castleWhiteKing = false;
    castleWhiteQueen = false;
    castleBlackKing = false;
    castleBlackQueen = false;
    enPassantSquare = -1;
    halfMoveClock = 0;
    fullMoveNumber = 1;
    whiteKingSquare = -1;
    blackKingSquare = -1;
    history.clear();
    moveHistory.clear();


    istringstream ss(fen);
    string piecePart, colorPart, castlePart, epPart;
    int halfMove, fullMove;

    ss >> piecePart >> colorPart >> castlePart >> epPart >> halfMove >> fullMove;


    int rank = 7;  
    int file = 0;  

    for (char c : piecePart) {
        if (c == '/') {

            rank--;
            file = 0;
        } else if (c >= '1' && c <= '8') {

            file += (c - '0');
        } else {

            Piece piece = NO_PIECE;
            switch (c) {
                case 'P': piece = W_PAWN;   break;
                case 'N': piece = W_KNIGHT; break;
                case 'B': piece = W_BISHOP; break;
                case 'R': piece = W_ROOK;   break;
                case 'Q': piece = W_QUEEN;  break;
                case 'K': piece = W_KING;   break;
                case 'p': piece = B_PAWN;   break;
                case 'n': piece = B_KNIGHT; break;
                case 'b': piece = B_BISHOP; break;
                case 'r': piece = B_ROOK;   break;
                case 'q': piece = B_QUEEN;  break;
                case 'k': piece = B_KING;   break;
            }
            int sq = makeSquare(rank, file);
            board[sq] = piece;


            if (piece == W_KING) whiteKingSquare = sq;
            if (piece == B_KING) blackKingSquare = sq;

            file++;
        }
    }


    sideToMove = (colorPart == "w") ? WHITE : BLACK;


    for (char c : castlePart) {
        if (c == 'K') castleWhiteKing = true;
        if (c == 'Q') castleWhiteQueen = true;
        if (c == 'k') castleBlackKing = true;
        if (c == 'q') castleBlackQueen = true;
    }


    if (epPart != "-") {
        enPassantSquare = stringToSquare(epPart);
    }


    halfMoveClock = halfMove;
    fullMoveNumber = fullMove;
}


void Board::makeMove(const Move& move) {

    UndoInfo undo;
    undo.capturedPiece = board[move.to];
    undo.capturedSquare = move.to;
    undo.castleWhiteKing = castleWhiteKing;
    undo.castleWhiteQueen = castleWhiteQueen;
    undo.castleBlackKing = castleBlackKing;
    undo.castleBlackQueen = castleBlackQueen;
    undo.enPassantSquare = enPassantSquare;
    undo.halfMoveClock = halfMoveClock;
    history.push_back(undo);

    Piece movingPiece = board[move.from];
    PieceType pt = pieceType(movingPiece);
    Color color = pieceColor(movingPiece);


    if (pt == PAWN || board[move.to] != NO_PIECE) {
        halfMoveClock = 0;
    } else {
        halfMoveClock++;
    }


    if (move.flag == FLAG_EN_PASSANT) {

        int capturedPawnSquare;
        if (color == WHITE) {
            capturedPawnSquare = move.to - 8;  
        } else {
            capturedPawnSquare = move.to + 8;  
        }
        board[capturedPawnSquare] = NO_PIECE;

        history.back().capturedPiece = makePiece(oppositeColor(color), PAWN);
        history.back().capturedSquare = capturedPawnSquare;
    }


    if (move.flag == FLAG_CASTLE) {

        int rookFrom, rookTo;
        if (move.to > move.from) {

            rookFrom = makeSquare(squareRank(move.from), 7);  
            rookTo   = makeSquare(squareRank(move.from), 5);  
        } else {

            rookFrom = makeSquare(squareRank(move.from), 0);  
            rookTo   = makeSquare(squareRank(move.from), 3);  
        }
        board[rookTo] = board[rookFrom];
        board[rookFrom] = NO_PIECE;
    }


    board[move.to] = movingPiece;
    board[move.from] = NO_PIECE;


    if (move.flag == FLAG_PROMOTION) {
        board[move.to] = makePiece(color, QUEEN);
    }


    if (pt == KING) {
        if (color == WHITE) {
            whiteKingSquare = move.to;
        } else {
            blackKingSquare = move.to;
        }
    }


    enPassantSquare = -1;
    if (pt == PAWN) {
        int rankDiff = squareRank(move.to) - squareRank(move.from);
        if (rankDiff == 2 || rankDiff == -2) {

            enPassantSquare = (move.from + move.to) / 2;
        }
    }


    if (pt == KING) {
        if (color == WHITE) {
            castleWhiteKing = false;
            castleWhiteQueen = false;
        } else {
            castleBlackKing = false;
            castleBlackQueen = false;
        }
    }

    if (move.from == 7 || move.to == 7)   castleWhiteKing = false;

    if (move.from == 0 || move.to == 0)   castleWhiteQueen = false;

    if (move.from == 63 || move.to == 63) castleBlackKing = false;

    if (move.from == 56 || move.to == 56) castleBlackQueen = false;


    if (sideToMove == BLACK) {
        fullMoveNumber++;
    }


    sideToMove = oppositeColor(sideToMove);


    moveHistory.push_back(move);
}


void Board::unmakeMove(const Move& move) {

    UndoInfo undo = history.back();
    history.pop_back();


    sideToMove = oppositeColor(sideToMove);

    if (sideToMove == BLACK) {
        fullMoveNumber--;
    }

    Piece movingPiece = board[move.to];


    if (move.flag == FLAG_PROMOTION) {

        movingPiece = makePiece(sideToMove, PAWN);
    }

    board[move.from] = movingPiece;
    board[move.to] = NO_PIECE;


    if (move.flag == FLAG_EN_PASSANT) {

        board[undo.capturedSquare] = undo.capturedPiece;
    } else {

        board[move.to] = undo.capturedPiece;
    }


    if (move.flag == FLAG_CASTLE) {
        int rookFrom, rookTo;
        if (move.to > move.from) {
            rookFrom = makeSquare(squareRank(move.from), 7);
            rookTo   = makeSquare(squareRank(move.from), 5);
        } else {
            rookFrom = makeSquare(squareRank(move.from), 0);
            rookTo   = makeSquare(squareRank(move.from), 3);
        }
        board[rookFrom] = board[rookTo];
        board[rookTo] = NO_PIECE;
    }


    if (pieceType(movingPiece) == KING) {
        if (sideToMove == WHITE) {
            whiteKingSquare = move.from;
        } else {
            blackKingSquare = move.from;
        }
    }


    castleWhiteKing = undo.castleWhiteKing;
    castleWhiteQueen = undo.castleWhiteQueen;
    castleBlackKing = undo.castleBlackKing;
    castleBlackQueen = undo.castleBlackQueen;
    enPassantSquare = undo.enPassantSquare;
    halfMoveClock = undo.halfMoveClock;


    if (!moveHistory.empty()) {
        moveHistory.pop_back();
    }
}


bool Board::isSquareAttacked(int square, Color byColor) const {
    int rank = squareRank(square);
    int file = squareFile(square);


    if (byColor == WHITE) {

        if (rank > 0) {
            if (file > 0 && board[makeSquare(rank - 1, file - 1)] == W_PAWN) return true;
            if (file < 7 && board[makeSquare(rank - 1, file + 1)] == W_PAWN) return true;
        }
    } else {

        if (rank < 7) {
            if (file > 0 && board[makeSquare(rank + 1, file - 1)] == B_PAWN) return true;
            if (file < 7 && board[makeSquare(rank + 1, file + 1)] == B_PAWN) return true;
        }
    }


    int knightOffsets[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        { 1, -2}, { 1, 2}, { 2, -1}, { 2, 1}
    };
    Piece enemyKnight = makePiece(byColor, KNIGHT);
    for (int i = 0; i < 8; i++) {
        int r = rank + knightOffsets[i][0];
        int f = file + knightOffsets[i][1];
        if (isValidRankFile(r, f) && board[makeSquare(r, f)] == enemyKnight) {
            return true;
        }
    }


    Piece enemyKing = makePiece(byColor, KING);
    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0) continue;  
            int r = rank + dr;
            int f = file + df;
            if (isValidRankFile(r, f) && board[makeSquare(r, f)] == enemyKing) {
                return true;
            }
        }
    }


    int diagDirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    Piece enemyBishop = makePiece(byColor, BISHOP);
    Piece enemyQueen  = makePiece(byColor, QUEEN);

    for (int d = 0; d < 4; d++) {
        for (int dist = 1; dist < 8; dist++) {
            int r = rank + diagDirs[d][0] * dist;
            int f = file + diagDirs[d][1] * dist;
            if (!isValidRankFile(r, f)) break;  

            Piece p = board[makeSquare(r, f)];
            if (p == NO_PIECE) continue;  
            if (p == enemyBishop || p == enemyQueen) return true;  
            break;  
        }
    }


    int straightDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    Piece enemyRook = makePiece(byColor, ROOK);

    for (int d = 0; d < 4; d++) {
        for (int dist = 1; dist < 8; dist++) {
            int r = rank + straightDirs[d][0] * dist;
            int f = file + straightDirs[d][1] * dist;
            if (!isValidRankFile(r, f)) break;

            Piece p = board[makeSquare(r, f)];
            if (p == NO_PIECE) continue;
            if (p == enemyRook || p == enemyQueen) return true;
            break;
        }
    }

    return false;  
}


bool Board::isInCheck(Color color) const {
    int kingSquare = (color == WHITE) ? whiteKingSquare : blackKingSquare;
    return isSquareAttacked(kingSquare, oppositeColor(color));
}


bool Board::isCheckmate(const vector<Move>& legalMoves) const {
    return legalMoves.empty() && isInCheck(sideToMove);
}


bool Board::isStalemate(const vector<Move>& legalMoves) const {
    return legalMoves.empty() && !isInCheck(sideToMove);
}


bool Board::isDraw() const {

    if (halfMoveClock >= 100) return true;


    int whiteKnights = 0, whiteBishops = 0, whiteOther = 0;
    int blackKnights = 0, blackBishops = 0, blackOther = 0;

    for (int i = 0; i < 64; i++) {
        Piece p = board[i];
        if (p == NO_PIECE) continue;
        PieceType pt = pieceType(p);
        Color c = pieceColor(p);
        if (pt == KING) continue;  

        if (c == WHITE) {
            if (pt == KNIGHT) whiteKnights++;
            else if (pt == BISHOP) whiteBishops++;
            else whiteOther++;
        } else {
            if (pt == KNIGHT) blackKnights++;
            else if (pt == BISHOP) blackBishops++;
            else blackOther++;
        }
    }


    if (whiteOther > 0 || blackOther > 0) return false;

    int whitePieces = whiteKnights + whiteBishops;
    int blackPieces = blackKnights + blackBishops;


    if (whitePieces == 0 && blackPieces == 0) return true;

    if (whitePieces == 0 && blackPieces == 1) return true;
    if (whitePieces == 1 && blackPieces == 0) return true;

    return false;
}


int Board::getKingSquare(Color color) const {
    return (color == WHITE) ? whiteKingSquare : blackKingSquare;
}


void Board::updateKingPosition(Piece piece, int square) {
    if (piece == W_KING) whiteKingSquare = square;
    if (piece == B_KING) blackKingSquare = square;
}


void Board::printBoard() const {
    cout << endl;
    cout << "    a   b   c   d   e   f   g   h" << endl;
    cout << "  +---+---+---+---+---+---+---+---+" << endl;


    for (int rank = 7; rank >= 0; rank--) {
        cout << (rank + 1) << " |";  

        for (int file = 0; file < 8; file++) {
            int sq = makeSquare(rank, file);
            Piece p = board[sq];

            if (p == NO_PIECE) {

                cout << "   |";
            } else {

                cout << " " << PIECE_LETTERS[p] << " |";
            }
        }
        cout << " " << (rank + 1) << endl;
        cout << "  +---+---+---+---+---+---+---+---+" << endl;
    }

    cout << "    a   b   c   d   e   f   g   h" << endl;
    cout << endl;


    cout << "  Turn: " << ((sideToMove == WHITE) ? "White" : "Black");
    cout << "  |  Move: " << fullMoveNumber;
    if (isInCheck(sideToMove)) {
        cout << "  |  ** CHECK! **";
    }
    cout << endl;
}


string Board::toFEN() const {
    string fen = "";

    const char pieceChars[] = {'P','N','B','R','Q','K','p','n','b','r','q','k'};
    for (int rank = 7; rank >= 0; rank--) {
        int emptyCount = 0;
        for (int file = 0; file < 8; file++) {
            Piece p = board[makeSquare(rank, file)];
            if (p == NO_PIECE) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += to_string(emptyCount);
                    emptyCount = 0;
                }
                fen += pieceChars[p];
            }
        }
        if (emptyCount > 0) {
            fen += to_string(emptyCount);
        }
        if (rank > 0) fen += '/';
    }


    fen += (sideToMove == WHITE) ? " w " : " b ";


    string castling = "";
    if (castleWhiteKing)  castling += 'K';
    if (castleWhiteQueen) castling += 'Q';
    if (castleBlackKing)  castling += 'k';
    if (castleBlackQueen) castling += 'q';
    if (castling.empty()) castling = "-";
    fen += castling;


    fen += " ";
    if (enPassantSquare >= 0) {
        fen += squareToString(enPassantSquare);
    } else {
        fen += "-";
    }


    fen += " " + to_string(halfMoveClock);


    fen += " " + to_string(fullMoveNumber);

    return fen;
}
