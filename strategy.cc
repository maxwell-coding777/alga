#include "strategy.h"
#include <fstream>
#include <string>

// Profondeur de recherche pour minMax / alphaBeta
const int NIVMAX = 4;
int stratChoice = 0;
int stratChoice2 = 0;
bool twoDiffIA = false;

void Strategy::applyMove (const movement& mv) {
        // Saut de 1 ou de 2
        int distance = std::max(std::abs(mv.nx - mv.ox), std::abs(mv.ny - mv.oy));

        int player = (int) _current_player;

        // Si distance de 1, duplication du blob
        if (distance <= 1) {
            _blobs.set(mv.nx, mv.ny, player);
        } else {
            _blobs.set(mv.ox, mv.oy, -1);
            _blobs.set(mv.nx, mv.ny, player);
        }

        // Action sur les blobs voisins
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) {
                    continue;
                }

                int nx = mv.nx + dx;
                int ny = mv.ny + dy;

                // validité de la case + pas un trou
                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                    if (_blobs.get(nx, ny) != -1 && !_holes.get(nx, ny) && _blobs.get(nx, ny) != player) {
                        _blobs.set(nx, ny, player);
                    }
                }
            }
        }
}

Sint32 Strategy::estimateCurrentScore () const {
    // ?
    Sint32 currScore = 0;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (_holes.get(x,y)) continue;
            if (_blobs.get(x,y) == (int) _current_player) {
                currScore++;
            } else if (_blobs.get(x,y) != -1) {
                currScore--;
            }
        }
    }
    return currScore;
}

// Sint32 Strategy::estimateCurrentScore () const {
//     // ?
//     Sint32 currScore = 0;

//     for (int x = 0; x < 8; x++) {
//         for (int y = 0; y < 8; y++) {
//             if (_holes.get(x,y)) continue;
//             if (_blobs.get(x,y) == (int) _current_player) {
//                 currScore++;
//                 if (x == 0 || x == 7 || y == 0 || y == 7) {
//                     currScore++;
//                 }
//                 for (int dx = -1; dx <= 1; dx++) {
//                     for (int dy = -1; dy <= 1; dy++) {
//                         int nx = x + dx;
//                         int ny = y + dy;
//                         if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
//                             if (_blobs.get(nx, ny) != -1 && _blobs.get(nx, ny) != (int)_current_player) {
//                                 currScore++;
//                             }
//                         }
//                     }
//                 }

//                 for (int dx = -2; dx <= 2; dx++) {
//                     for (int dy = -2; dy <= 2; dy++) {
//                         int nx = x + dx;
//                         int ny = y + dy;
//                         if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8 && _blobs.get(nx, ny) == -1 && !_holes.get(nx, ny)) {
//                             currScore++;
//                         }
//                     }
//                 }
//             } else if (_blobs.get(x,y) != -1) {
//                 currScore--;
//             }
//         }
//     }
//     return currScore;
// }

vector<movement>& Strategy::computeValidMoves(vector<movement>& valid_moves) const {

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (_blobs.get(x, y) == (int)_current_player) {
                for (int nx = std::max(0, x - 2); nx <= std::min(7, x + 2); nx++) {
                    for (int ny = std::max(0, y - 2); ny <= std::min(7, y + 2); ny++) {
                        if (_blobs.get(nx, ny) == -1 && !_holes.get(nx, ny)) {
                            if (!(x == nx && y == ny)) {
                                valid_moves.push_back(movement(x, y, nx, ny));
                            }
                        }
                    }
                }
            }
        }
    }
    return valid_moves;
}

void Strategy::logToCSV(const std::string& method_name, Sint32 evaluation) {
    std::ofstream file(method_name + ".csv", std::ios::app);
    if (file.is_open()) {
        file << evaluation << "\n";
        file.close();
    }
}

void Strategy::naiveMove() {

    movement mv(0,0,0,0);

    for(mv.ox = 0 ; mv.ox < 8 ; mv.ox++) {
        for(mv.oy = 0 ; mv.oy < 8 ; mv.oy++) {
            if (_blobs.get(mv.ox, mv.oy) == (int) _current_player) {
                //iterate on possible destinations
                for(mv.nx = std::max(0,mv.ox-2) ; mv.nx <= std::min(7,mv.ox+2) ; mv.nx++) {
                    for(mv.ny = std::max(0,mv.oy-2) ; mv.ny <= std::min(7,mv.oy+2) ; mv.ny++) {
                        if (_holes.get(mv.nx, mv.ny)) continue;
                        if (_blobs.get(mv.nx, mv.ny) == -1) goto end_choice;
                    }
                }
            }
        }
    }
    
    end_choice:
        _saveBestMove(mv);
        return;
}

void Strategy::gloutonMove() {

    std::vector<movement> valid_moves;
    Strategy::computeValidMoves(valid_moves);

    Sint32 eval = INT32_MIN;
    movement bestMove(0,0,0,0);

    for (const movement& mov : valid_moves) {
        Strategy temp = *this;
        temp.applyMove(mov);

        Sint32 newEval = temp.estimateCurrentScore();

        if (newEval > eval) {
            eval = newEval;
            bestMove = mov;
        }
    }
    
    _saveBestMove(bestMove);
    return;
}

Sint32 Strategy::minMaxMove(int niveau, int i) {
    if (niveau == NIVMAX) {
        Sint32 score = Strategy::estimateCurrentScore() * i;
        return score;
    }
    std::vector<movement> valid_moves;
    Strategy::computeValidMoves(valid_moves);
    if (valid_moves.empty()) {
        return Strategy::estimateCurrentScore() * i;
    }
    Sint32 eval = (i == 1) ? INT32_MIN : INT32_MAX;
    movement bestMove = valid_moves[0];
    for (const movement& mov : valid_moves) {
        bidiarray<Sint16> currentState = _blobs;
        Strategy::applyMove(mov);
        _current_player = 1 - _current_player;
        Sint32 score = Strategy::minMaxMove(niveau + 1, -i);
        _current_player = 1 - _current_player;
        _blobs = currentState;
        if (i * score > i * eval) {
            eval = score;
            bestMove = mov;
        }
    }
    if (niveau == 0) {
        _saveBestMove(bestMove);
    }
    return eval;
}

Sint32 Strategy::minMaxParaMove(int niveau, int i) {
    // Essai de parallélisation niveau 0 : un peu guez
    if (niveau == 0) {
        std::vector<movement> valid_moves;
        Strategy::computeValidMoves(valid_moves);
        
        if (valid_moves.empty()) {
            return Strategy::estimateCurrentScore();
        }
        
        Sint32 best_eval = i * INT32_MIN;
        movement global_best_move = valid_moves[0];
        _saveBestMove(global_best_move);
        
        #pragma omp parallel
        {
            Sint32 local_best_eval = i * INT32_MIN;
            movement local_best_move = valid_moves[0];
            
            #pragma omp for nowait
            for (size_t m = 0; m < valid_moves.size(); m++) {
                const movement& mov = valid_moves[m];
                
                bidiarray<Sint16> thread_state = _blobs;
                
                Strategy::applyMove(mov);
                Sint32 newEval = Strategy::minMaxMove(1, -i);
                if (i * newEval > i * local_best_eval) {
                    local_best_eval = newEval;
                    local_best_move = mov;
                }
                
                _blobs = thread_state;
            }
            
            #pragma omp critical
            {
                if (i * local_best_eval > i * best_eval) {
                    best_eval = local_best_eval;
                    global_best_move = local_best_move;
                    _saveBestMove(global_best_move);
                }
            }
        }
        
        return best_eval;
    }
    else {
        Sint32 eval;
        
        if (niveau == NIVMAX) {
            return Strategy::estimateCurrentScore();
        }
        
        std::vector<movement> valid_moves;
        Strategy::computeValidMoves(valid_moves);
        
        if (valid_moves.empty()) {
            return Strategy::estimateCurrentScore();
        }
        
        eval = i * INT32_MIN;
        
        for (const movement& mov : valid_moves) {
            bidiarray<Sint16> currState = _blobs;
            
            Strategy::applyMove(mov);
            Sint32 newEval = Strategy::minMaxMove(niveau + 1, -i);
            
            if (i * newEval > i * eval) {
                eval = newEval;
            }
            
            _blobs = currState;
        }
        
        return eval;
    }
}
Sint32 Strategy::alphaBetaMove(int niveau, int i, Sint32 alpha, Sint32 beta) {
    if (niveau == NIVMAX) {
        Sint32 score = Strategy::estimateCurrentScore() * i;
        return score;
    }
    std::vector<movement> valid_moves;
    Strategy::computeValidMoves(valid_moves);
    if (valid_moves.empty()) {
        return Strategy::estimateCurrentScore() * i;
    }
    Sint32 eval = (i == 1) ? INT32_MIN : INT32_MAX;
    movement bestMove = valid_moves[0];
    for (const movement& mov : valid_moves) {
        bidiarray<Sint16> currentState = _blobs;
        Strategy::applyMove(mov);
        _current_player = 1 - _current_player;
        Sint32 score = Strategy::alphaBetaMove(niveau + 1, -i, alpha, beta);
        _current_player = 1 - _current_player;
        _blobs = currentState;
        if (i * score > i * eval) {
            eval = score;
            bestMove = mov;
        }
        if (i == 1) {
            alpha = std::max(alpha, eval);
        } else {
            beta = std::min(beta, eval);
        }
        if (beta <= alpha) {
            break;
        }
    }
    if (niveau == 0) {
        _saveBestMove(bestMove);
    }
    return eval;
}

Sint32 Strategy::alphaBetaParaMove(int niveau, int a, int b) {
    movement bestMove;
    std::vector<movement> valid_moves;
    Strategy::computeValidMoves(valid_moves);
    Sint32 eval;

    // Condition terminale
    if (niveau == NIVMAX || valid_moves.empty()) {
        return this->estimateCurrentScore();
    }

    // Parallélisation du niveau 0
    if (niveau == 0) {
        #pragma omp parallel 
        {
            Sint32 maxEval = INT32_MIN;

            #pragma omp for nowait
            for (const movement& mov : valid_moves) {
                if (b <= a) continue;
                Strategy localStrat = *this;
                
                this->applyMove(mov);
                this->alphaBetaMove(niveau+1, 1,a, b);
                eval = this->estimateCurrentScore();

                #pragma omp critical 
                {
                    if (eval > maxEval) {
                        maxEval = eval;
                        bestMove = mov;
                        a = std::max(a, eval);
                        _saveBestMove(bestMove);
                    }
                }
            }
        }
    }
    // Noeud max
    else if (niveau % 2 == 0) {
        Sint32 maxEval = INT32_MIN;

        for (const movement& mov : valid_moves) {

            bidiarray<Sint16> currState = _blobs;
            // Strategy temp = *this;
            applyMove(mov);
            
            alphaBetaMove(niveau+1, 1 ,a, b);
            
            eval = estimateCurrentScore();

            if (eval > maxEval) {
                maxEval = eval;
                bestMove = mov;
                a = std::max(a, eval);
            }

            _blobs = currState;

            if (b <= a) {
                break; // Élagage beta
            }
        }
        // Noeud min
    } else {
        Sint32 minEval = INT32_MAX;

        for (const movement& mov : valid_moves) {

            bidiarray<Sint16> currState = _blobs;
            // Strategy temp = *this;
            applyMove(mov);
            
            movement childMove;
            alphaBetaMove(niveau + 1, 1,a, b);
            
            eval = estimateCurrentScore();
            if (eval < minEval) {
                minEval = eval;
                bestMove = mov;
                b = std::min(b, eval);
            }

            _blobs = currState;

            if (b <= a) {
                break; // Élagage alpha
            }
        }
    }
    return eval;
}

movement Strategy::findNearestAlly(int x, int y) const {
    for(int dx = std::max(0, x-1); dx <= std::min(7, x+1); dx++) {
        for (int dy = std::max(0, y-1); dy <= std::min(7, y+1); dy++) {
            if (_holes.get(dx, dy)) continue;
            if (x == dx && y == dy) continue;
            if (_blobs.get(dx, dy) == (int) _current_player) return movement(dx, dy, x, y);
        }
    }
    return movement(0, 0, 0, 0); // Null if no friends ;(
}

vector<movement>& Strategy::computeValidMovesP1(vector<movement>& valid_moves, int size) const {
    // Expand from bottom-right corner (7,7)

    int top = 7 - size;
    int left = 7 - size;

    if (top < 0 || left < 0) {
        if (left >= 0) {
            // Expand left side
            for (int y = 0; y <= 7; y++) {
                if (_blobs.get(left, y) == -1 && !_holes.get(left, y)) {
                    movement mv = findNearestAlly(left, y);
                    if (!mv.isNull()) valid_moves.push_back(mv);
                }
            }
        } else {
            // Expansion classic right side
            for (int x = 0; x <= 7; x++) {
                for (int y = 0; y <= 7; y++) {
                    if (_blobs.get(x, y) == -1 && !_holes.get(x, y)) {
                        movement mv = findNearestAlly(x, y);
                        if (!mv.isNull()) valid_moves.push_back(mv);
                    }
                }
            }
        }
        return valid_moves;
    }

    // Top edge
    for (int x = left; x <= 7; x++) {
        if (_blobs.get(x, top) == -1 && !_holes.get(x, top)) {
            movement mv = findNearestAlly(x, top);
            if (!mv.isNull()) valid_moves.push_back(mv);
        }
    }

    // Left edge
    for (int y = top + 1; y <= 7; y++) {
        if (_blobs.get(left, y) == -1 && !_holes.get(left, y)) {
            movement mv = findNearestAlly(left, y);
            if (!mv.isNull()) valid_moves.push_back(mv);
        }
    }

    return valid_moves;
}

vector<movement>& Strategy::computeValidMovesP0(vector<movement>& valid_moves, int size) const {
    // Expand from top-left corner (0,0)

    int bottom = 0 + size;
    int right = 0 + size;

    if (bottom > 7 || right > 7) {
        if (right <= 7) {
            // Right side
            for (int y = 0; y <= 7; y++) {
                if (_blobs.get(right, y) == -1 && !_holes.get(right, y)) {
                    movement mv = findNearestAlly(right, y);
                    if (!mv.isNull()) valid_moves.push_back(mv);
                }
            }
        } else {
            // Left side
            for (int x = 0; x <= 7; x++) {
                for (int y = 0; y <= 7; y++) {
                    if (_blobs.get(x, y) == -1 && !_holes.get(x, y)) {
                        movement mv = findNearestAlly(x, y);
                        if (!mv.isNull()) valid_moves.push_back(mv);
                    }
                }
            }
        }
        return valid_moves;
    }

    // Bottom edge
    for (int x = 0; x <= right; x++) {
        if (_blobs.get(x, bottom) == -1 && !_holes.get(x, bottom)) {
            movement mv = findNearestAlly(x, bottom);
            if (!mv.isNull()) valid_moves.push_back(mv);
        }
    }

    // Right edge
    for (int y = 0; y < bottom; y++) {
        if (_blobs.get(right, y) == -1 && !_holes.get(right, y)) {
            movement mv = findNearestAlly(right, y);
            if (!mv.isNull()) valid_moves.push_back(mv);
        }
    }

    return valid_moves;
}

void Strategy::classicMove() {
    // Strategy to expand based on the current player
    // Classic car plie classique sur classique
    std::vector<movement> availableMov;

    int maxTries = 16; // Au cas où
    int size = 0;

    if ((int) _current_player == 1) {
        // Player 1: Expand from bottom-right (7,7)
        while (availableMov.empty() && maxTries > 0) {
            computeValidMovesP1(availableMov, size);
            size++;
            maxTries--;
        }
    } else {
        // Player 0: Expand from top-left (0,0)
        while (availableMov.empty() && maxTries > 0) {
            computeValidMovesP0(availableMov, size);
            size++;
            maxTries--;
        }
    }

    if (availableMov.empty()) {
        //Strategy::gloutonMove(); // Au pire Glouton
	Strategy::alphaBetaMove(0, 1, INT32_MIN, INT32_MAX);
        return;
    }

    // Glouton on available moves
    movement bestMove;
    int eval = INT32_MIN;

    for (const movement& mov : availableMov) {
        bidiarray<Sint16> currState = _blobs;

        Strategy::applyMove(mov);
        Sint32 newEval = Strategy::estimateCurrentScore();

        _blobs = currState;

        if (newEval > eval) {
            eval = newEval;
            bestMove = mov;
        }
    }

    _saveBestMove(bestMove);
    logToCSV("classicMove", eval);
}

void Strategy::strategyMove(int choice) {
    if (choice == 1) {
        // Stratégie glouton
		printf("Glouton choice : ");
        Strategy::gloutonMove();
    } else if (choice == 2) {
        // Stratégie min-max
		printf("minMaxMove choice : ");
        Strategy::minMaxMove(0, 1);
    } else if (choice == 3) {
        // Stratégie min-max parallel
		printf("minMaxParaMove choice : ");
        Strategy::minMaxParaMove(0, 1);
    } else if (choice == 4) {
        // Stratégie alpha-beta
		printf("alphaBetaMove choice : ");
        Strategy::alphaBetaMove(0, 1, INT32_MIN, INT32_MAX);
    } else if (choice == 5) {
		printf("alphaBetaParaMove choice : ");
        Strategy::alphaBetaParaMove(0, INT32_MIN, INT32_MAX);
    } else if (choice == 6) {
        // Stratégie perso
        printf("Personnal choice : ");
        Strategy::classicMove();
    } else {
		printf("naiveMove choice : ");
        Strategy::naiveMove();
    }
    logToCSV("strategyMove", estimateCurrentScore());
    printf("player %d\n", (int) _current_player);
}
