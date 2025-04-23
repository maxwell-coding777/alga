#ifndef __STRATEGY_H
#define __STRATEGY_H

#include "common.h"
#include "bidiarray.h"
#include "move.h"

#include <thread>
#include <vector>
#include <mutex>
#include <limits>
#include <omp.h>


class Strategy {

private:
    //! array containing all blobs on the board
    bidiarray<Sint16> _blobs;
    //! an array of booleans indicating for each cell whether it is a hole or not.
    const bidiarray<bool>& _holes;
    //! Current player
    Uint16 _current_player;

    //! Call this function to save your best move.
    //! Multiple call can be done each turn,
    //! Only the last move saved will be used.
    void (*_saveBestMove)(movement&);

public:
        // Constructor from a current situation
    Strategy (bidiarray<Sint16>& blobs, 
              const bidiarray<bool>& holes,
              const Uint16 current_player,
              void (*saveBestMove)(movement&))
            : _blobs(blobs),_holes(holes), _current_player(current_player), _saveBestMove(saveBestMove)
        {
        }
    
              
    
        // Copy constructor
    Strategy (const Strategy& St)
            : _blobs(St._blobs), _holes(St._holes),_current_player(St._current_player) 
        {}
    
        // Destructor
    ~Strategy() {}
    
        /** 
         * Apply a move to the current state of blobs
         * Assumes that the move is valid
         */
    void applyMove (const movement& mv);

        /**
         * Compute the vector containing every possible moves
         */
    vector<movement>& computeValidMoves (vector<movement>& valid_moves) const;

        /**
         * Estimate the score of the current state of the game
         */
    Sint32 estimateCurrentScore () const;

        /**
         * Use the first move available
         */
    void naiveMove();

        /**
         * Find the best move using a glouton algorithm
         */
    void gloutonMove();

        /**
         * Find the best move using a min max algorithm
         */
    Sint32 minMaxMove(int niveau, int i);

        /**
         * Same but parallel
         */
    Sint32 minMaxParaMove(int niveau, int i);

        /**
         * Find the best move using an alpha beta algorithm
         */
    Sint32 alphaBetaMove(int niveau, int a, int b);


        /**
         *  Find the best move using a parallel alpha beta algorithm
         */
    Sint32 alphaBetaParaMove(int niveau, int a, int b);

        /**
         * Find the best move.
         */
    void computeBestMove();
    
        /**
         * Same thing but to be used in fork (avoid use of global var)
         */
    void strategyMove(int choice);

    // La c'est mon bouiboui
    movement findNearestAlly(int x, int y) const;
    vector<movement>& computeValidMovesP0(vector<movement>& valid_moves, int size) const;
    vector<movement>& computeValidMovesP1(vector<movement>& valid_moves, int size) const;
    void classicMove();
    
};

#endif
