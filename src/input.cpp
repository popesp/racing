#include	"input.h"

#include	<gl/glew.h>
#include	<GLFW/glfw3.h>


/*	start up the texture manager
	param:	im				input manager (modified)
*/
void inputmanager_startup(struct inputmanager* im)
{
	int i;

	// clear all controller flags and initialize attributes
	for (i = 0; i < INPUT_MAX_JOYSTICKS; i++)
	{
		im->controllers[i].flags = INPUT_FLAG_NONE;

		im->controllers[i].num_buttons = 0;
		im->controllers[i].buttons = NULL;

		im->controllers[i].num_axes = 0;
		im->controllers[i].axes = NULL;
	}

	inputmanager_update(im);
}

/*	shut down the input manager
	param:	im				input manager (modified)
*/
void inputmanager_shutdown(struct inputmanager* im)
{
	(void)im;
}


/*	update the input
	param:	im				input manager (modified)
*/
void inputmanager_update(struct inputmanager* im)
{
	int i;

	for (i = 0; i < INPUT_MAX_JOYSTICKS; i++)
	{
		if (glfwJoystickPresent(GLFW_JOYSTICK_1+i))
		{
			if (!(im->controllers[i].flags & INPUT_FLAG_ENABLED))
			{} // controller was connected

			im->controllers[i].flags |= INPUT_FLAG_ENABLED;
			im->controllers[i].buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1+i, &im->controllers[i].num_buttons);
			im->controllers[i].axes = (float*)glfwGetJoystickAxes(GLFW_JOYSTICK_1+i, &im->controllers[i].num_axes);

			// left stick deadzone
			if ((im->controllers[i].axes[INPUT_AXIS_LEFT_LR]*im->controllers[i].axes[INPUT_AXIS_LEFT_LR] + im->controllers[i].axes[INPUT_AXIS_LEFT_UD]*im->controllers[i].axes[INPUT_AXIS_LEFT_UD]) < INPUT_ANALOG_DEADZONE)
			{
				im->controllers[i].axes[INPUT_AXIS_LEFT_LR] = 0.f;
				im->controllers[i].axes[INPUT_AXIS_LEFT_UD] = 0.f;
			}

			// right stick deadzone
			if ((im->controllers[i].axes[INPUT_AXIS_RIGHT_LR]*im->controllers[i].axes[INPUT_AXIS_RIGHT_LR] + im->controllers[i].axes[INPUT_AXIS_RIGHT_UD]*im->controllers[i].axes[INPUT_AXIS_RIGHT_UD]) < INPUT_ANALOG_DEADZONE)
			{
				im->controllers[i].axes[INPUT_AXIS_RIGHT_LR] = 0.f;
				im->controllers[i].axes[INPUT_AXIS_RIGHT_UD] = 0.f;
			}
		} else
		{
			if (im->controllers[i].flags & INPUT_FLAG_ENABLED)
			{} // controller was disconnected

			im->controllers[i].flags &= ~INPUT_FLAG_ENABLED;
		}
	}
}


/*	get the name of a given joystick
	param:	im				input manager
	param:	id_joyst		id of the joystick to query
	return:	const char*		name of the joystick
*/
const char* inputmanager_joystickname(struct inputmanager* im, int id_joyst)
{
	(void)im;
	return glfwGetJoystickName(GLFW_JOYSTICK_1 + id_joyst);
}