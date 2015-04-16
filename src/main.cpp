#include	"core/game.h"


int main(int argc, char** argv)
{
	struct game game;

	(void)argc;
	(void)argv;

	if (!game_startup(&game))
		return 0;

	game_mainloop(&game);
	game_shutdown(&game);

	return 0;
}