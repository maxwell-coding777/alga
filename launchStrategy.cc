#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "strategy.h"
#include "shmem.h"

//! Display a move on console
void saveBestMoveToConsole(movement& m)
{
	cout<<"SAVE MOVE: "<<(Uint32)m.ox<<","<<(Uint32)m.oy<<" to "<<(Uint32)m.nx<<","<<(Uint32)m.ny<<endl;
}

//! Save a move to the shared memory with blobwar
void saveBestMoveToShmem(movement& m)
{
	// Uncomment this to show debug messages
// #ifdef DEBUG
// 	saveBestMoveToConsole(m);
// #endif
	shmem_set(m);
}

/** Main of launchStrategy
 * This executable is called automatically by blobwar to compute IA moves.
 * The args should be:
 * - blobs (serialized)
 * - holes (serialized)
 * - current player (an int)
 * - chosen strategy (an int)
 */
int main(int argc, char **argv) {
	
#ifdef DEBUG
	cout << "Starting launchStrategy" << endl;
#endif
	if(argc != 5) {
		printf("Usage: ./launchStrategy blobs holes current_player strategy_number\n");
		printf("	blobs is a serialized bidiarray<Sint16> containing the blobs\n");
		printf("	holes is a serialized bidiarray<bool> containing the holes\n");
		printf("	current_player is an int indicating which player should play\n");
		printf("	strategy_number is an int indicating which strategy will be used :\n");
		printf(" 	0 : naive; 1 : glouton; 2 : minmax: 3 : alpha-beta; 4 : alpha-beta parralel\n.");
		return 1;
	}
	int i = 1;
	
	bidiarray<Sint16> blobs = bidiarray<Sint16>::deserialize(argv[i++]);
	bidiarray<bool> holes = bidiarray<bool>::deserialize(argv[i++]);
	int cplayer = atoi(argv[i++]);
	stratChoice = atoi(argv[i++]);

#ifdef DEBUG_PERSO
	blobs.display();
	holes.display();
	std::cout << "player: "<<cplayer<<std::endl;
#endif

	void (*func)(movement&) = saveBestMoveToShmem;
	
	shmem_init();
	func = saveBestMoveToShmem;
	
	Strategy strategy(blobs, holes, cplayer, func);
	strategy.strategyMove(stratChoice);

#ifdef DEBUG_PERSO
	movement m = shmem_get();
	int ox = m.ox;
	int oy = m.oy;
	int nx = m.nx;
	int ny = m.ny;
	cout<<"move from ("<< cplayer << "): " <<(Uint32)ox<<","<<(Uint32)oy<<" to "<<(Uint32)nx<<","<<(Uint32)ny<<endl;
#endif
	
	return 0;
}




