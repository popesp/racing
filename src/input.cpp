#include	"input.h"

#include	<gl/glew.h>
#include	<GLFW/glfw3.h>


void input_startup(struct inputmanager* input)
{
	int i;

	// clear all controller flags and initialize attributes
	for (i = 0; i < INPUT_MAX_JOYSTICKS; i++)
	{
		input->controllers[i].flags = INPUT_FLAG_NONE;

		input->controllers[i].num_buttons = 0;
		input->controllers[i].buttons = NULL;

		input->controllers[i].num_axes = 0;
		input->controllers[i].axes = NULL;
	}

	input_update(input);
}


void input_update(struct inputmanager* input)
{
	int i;

	for (i = 0; i < INPUT_MAX_JOYSTICKS; i++)
	{
		if (glfwJoystickPresent(GLFW_JOYSTICK_1+i))
		{
			if (!(input->controllers[i].flags & INPUT_FLAG_ENABLED))
			{} // controller was connected

			input->controllers[i].flags |= INPUT_FLAG_ENABLED;
			input->controllers[i].buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1+i, &input->controllers[i].num_buttons);
			input->controllers[i].axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1+i, &input->controllers[i].num_axes);
		} else
		{
			if (input->controllers[i].flags & INPUT_FLAG_ENABLED)
			{} // controller was disconnected

			input->controllers[i].flags &= ~INPUT_FLAG_ENABLED;
		}
	}
}

const char* input_joystickname(int joystick)
{
	return glfwGetJoystickName(GLFW_JOYSTICK_1+joystick);
}