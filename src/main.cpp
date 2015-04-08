#include	"core/game.h"
#include	"core/menu.h"
#include	"error.h"			// PRINT_ERROR

int main(int argc, char** argv)
{
	struct game game;

	(void)argc;
	(void)argv;

	if (!start_subsystems(&game))
	{
		PRINT_ERROR("Problem starting game subsystems.\n");
		return 0;
	}

	game.num_aiplayers = GAME_INIT_AI;
	game.num_laps = GAME_INIT_LAPS;
	game.soundon = GAME_INIT_SOUNDON;
	game.countdown = GAME_INIT_COUNTDOWN;
	game.difficulty = GAME_INIT_DIFFICULTY;

	if (!menu_startup(&game))
		return 0;

	if(!(game.menuflags & MENU_FLAG_SHUTDOWN)){
		if (!game_startup(&game))
		return 0;

		game_mainloop(&game);
		game_shutdown(&game);
	}

	return 0;
}