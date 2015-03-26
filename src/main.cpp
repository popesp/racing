#include	"core/game.h"
#include	"core/ui.h"
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