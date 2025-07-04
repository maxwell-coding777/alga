#include"blobwar.h"

/**
 *	\mainpage Documentation for blobwar
 *
 * 	This doxygen manual documents the 
 * 	code of the blobwar game and the 
 * 	most important data structures.
 *
 *	\section game Understanding the game
 *
 * 	Blobwar is a turn by turn strategy game in
 * 	which each player
 * 	controls colored blobs : small spherical
 * 	entities ; and tries to win by finishing 
 * 	with more blobs than any of his opponents.
 *
 * 	At each turn, a player can select one of his blobs
 * 	and duplicate him on an adjacent cell
 * 	of the board (diagonal movements allowed) or move him
 * 	by making him jump to a cell at a distance of 2.
 *
 * 	After the move (or the copy), the blob will convert
 * 	any adjacent blob to his color. A small counter at the
 * 	bottom of the screen keeps track of the scores of all
 * 	players. To increase the interest of the game, 
 * 	some boards cells are "holes" and no blob can enter them.
 * 	Different boards with different patterns of holes are
 * 	available to play with.
 *
 * 	When a player cannot move, he loses his turn.
 * 	When no player can move any more, the game finishes.
 * 	The player with the highest number of blobs is then
 * 	the winner.
 *
 *	\section structures Useful data structures to program the AI
 *
 * 	In this section, we quickly explain what files are the most
 * 	valuable to anyone designing an artificial intelligence.
 *	First of all, all modifications should fit in the strategy.cc
 *	file. In fact, the artificial intelligence is called
 *	by the method computeBestMove() of the strategy class.
 *	When this method returns, the function _saveBestMove should
 *	have been called with the coordinates of the move (from old board cell
 *	to new board cell).
 *
 *	To compute the best move to play, several structures are needed:
 *	-# Strategy::_holes is an array of booleans indicating 
 *	for each cell whether it is a hole or not.
 *	-# Strategy::_current_player is the number of the player who is playing
 *	-# Strategy::_blobs is the array containing all blobs on the board.
 *
 * */

/** this is the main game variable */
blobwar *game;

int main(int argc, char **argv)
{
	int compute_time_IA = 0;
	if(argc > 1) {
		for(int i = 1; i < argc; i++) {
			if(strcmp(argv[i], "-h") == 0) {
				printf("usage: ./blobwar [-t <time>] [-strategy1] [-strategy2]\n");
				printf("    -t <time>       let IA compute during <time> (default: 1).\n");
				printf("    -glouton        force IA to use a glouton strategy.\n");
				printf("    -minmax         force IA to use a minmax strategy.\n");
				printf("    -minmaxpara     force IA to use a parallel minmax strategy.\n");
				printf("    -alphabeta      force IA to use an alpha-beta strategy.\n");
				printf("    -abpara         force IA to use an alpha-beta parallel strategy.\n");
                printf("    -personal       for IA to use our personal strategy.\n");
				printf("You can also specify a second strategy to make a match between them.\n");
				printf("If no option is specified, a naive algorithm will be used.\n");
				exit(0);
			}
			else if(strcmp(argv[i], "-t") == 0) {
				if(i + 1 < argc) {
					compute_time_IA = atoi(argv[++i]);
				} else {
					printf("Error: -t option requires a time argument.\n");
					exit(1);
				}
			}
			else if(strcmp(argv[i], "-glouton") == 0) {
				if(stratChoice == 0) {
					printf("Chosen strategy (blue): glouton.\n");
					stratChoice = 1;
				} else if(stratChoice2 == 0) {
					printf("Second chosen strategy (red): glouton.\n");
					stratChoice2 = 1;
				}
			}
			else if(strcmp(argv[i], "-minmax") == 0) {
				if(stratChoice == 0) {
					printf("Chosen strategy (blue): Min-Max.\n");
					stratChoice = 2;
				} else if(stratChoice2 == 0) {
					printf("Second chosen strategy (red): Min-Max.\n");
					stratChoice2 = 2;
				}
			}
			else if(strcmp(argv[i], "-minmaxpara") == 0) {
				if(stratChoice == 0) {
					printf("Chosen strategy (blue): Min-Max Parallel.\n");
					stratChoice = 3;
				} else if(stratChoice2 == 0) {
					printf("Second chosen strategy (red): Min-Max Parallel.\n");
					stratChoice2 = 3;
				}
			}
			else if(strcmp(argv[i], "-alphabeta") == 0) {
				if(stratChoice == 0) {
					printf("Chosen strategy (blue): Alpha-Beta.\n");
					stratChoice = 4;
				} else if(stratChoice2 == 0) {
					printf("Second chosen strategy (red): Alpha-Beta.\n");
					stratChoice2 = 4;
				}
			}
			else if(strcmp(argv[i], "-abpara") == 0) {
				if(stratChoice == 0) {
					printf("Chosen strategy (blue): Alpha-Beta Parallel.\n");
					stratChoice = 5;
				} else if(stratChoice2 == 0) {
					printf("Second chosen strategy (red): Alpha-Beta Parallel.\n");
					stratChoice2 = 5;
				}
			}
            else if(strcmp(argv[i], "-personal") == 0) {
                if(stratChoice == 0) {
                    printf("Chosen strategy (blue): Personal.\n");
                    stratChoice = 6;
                } else if(stratChoice2 == 0) {
                    printf("Second chosen strategy (red): Personal.\n");
                    stratChoice2 = 6;
                }
            }
			else {
				printf("Unknown option: %s\n", argv[i]);
				printf("Use ./blobwar -h for help.\n");
				exit(1);
			}
		}
	}

	if(compute_time_IA <= 0) compute_time_IA = 1;
	
	
	Uint32 new_ticks, diff;
#ifdef DEBUG
	cout << "Starting game" << endl;
#endif
	
	//open video, sound, bugs buffer, ....
	game = new blobwar();
	game->compute_time_IA = compute_time_IA;

	//what time is it doc ?
	game->ticks = SDL_GetTicks();

	//now enter main game loop 
	while (true) {
		//handle the game (or try to)
		game->handle();

		//we don't update the screen now
		//wait until we are synchronized

		//how many time elapsed ?
		new_ticks = SDL_GetTicks();
		diff = new_ticks - game->ticks;
		//were we quick enough ??
		if (diff < (1000/game->framerate)) {
			//yes, haha i've got an 3.4Ghz PC
			//let's give the extra time back to linux
			SDL_Delay((1000/game->framerate) - diff);
			game->frame++;
			game->ticks = game->ticks + (1000/game->framerate);
#ifdef ANIMATION
			//if we have some animations, update display 
			if ((game->frame % ANIMATIONSPEED)==0) game->display2update = true;
#endif
		} else {
			//no, this bloated box would go faster on wheels
			SDL_Delay((1000/game->framerate) - (diff % (1000/game->framerate)));
			game->frame += 1 + (Uint32) (diff / (1000/game->framerate));
			game->ticks = game->ticks + (1000/game->framerate) * (Uint32) ((diff / (1000/game->framerate)) + 1);
		}

		//update the screen
		game->update();

	}
	return 0;
}
