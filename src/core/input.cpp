#include	"input.h"

#include	<gl/glew.h>
#include	<GLFW/glfw3.h>
#include	<stdio.h>				// printf
#include	"../mem.h"
#include	"../render/window.h"


void inputmanager_startup(struct inputmanager* im, struct window* win)
{
	int i;

	im->win = win;

	// clear all controller flags and initialize attributes
	for (i = 0; i < INPUT_MAX_JOYSTICKS; i++)
	{
		im->controllers[i].flags = INPUT_FLAG_NONE;

		im->controllers[i].num_buttons = 0;
		im->controllers[i].buttons = NULL;

		im->controllers[i].num_axes = 0;
		im->controllers[i].axes = NULL;
	}


	// initialize keyboard
	im->keyboard.num_buttons = INPUT_CONTROLLER_BUTTONS;
	im->keyboard.num_axes = INPUT_CONTROLLER_AXES;

	im->keyboard.buttons = (unsigned char*)mem_alloc(sizeof(unsigned char) * im->keyboard.num_buttons);
	for (i = 0; i < im->keyboard.num_buttons; i++)
		im->keyboard.buttons[i] = INPUT_STATE_INIT;

	im->keyboard.axes = (float*)mem_alloc(sizeof(float) * im->keyboard.num_axes);
	for (i = 0; i < im->keyboard.num_axes; i++)
		im->keyboard.axes[i] = 0.f;

	im->keyboard.flags = INPUT_FLAG_ENABLED;

	inputmanager_update(im);
}

void inputmanager_shutdown(struct inputmanager* im)
{
	struct controller* c;
	int i;

	c = im->controllers;
	for (i = 0; i < INPUT_MAX_JOYSTICKS; i++, c++)
	{
		if (c->flags & INPUT_FLAG_ENABLED)
		{
			mem_free(c->buttons);
			mem_free(c->axes);
		}
	}

	mem_free(im->keyboard.buttons);
	mem_free(im->keyboard.axes);
}


void inputmanager_update(struct inputmanager* im)
{
	struct controller* c;
	const unsigned char* buttons;
	const float* axes;
	int i, j;

	c = im->controllers;
	for (i = 0; i < INPUT_MAX_JOYSTICKS; i++, c++)
	{
		if (glfwJoystickPresent(GLFW_JOYSTICK_1+i))
		{
			buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1 + i, &c->num_buttons);
			axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1 + i, &c->num_axes);

			// if controller was just connected, allocate arrays for the button and axis states
			if (!(c->flags & INPUT_FLAG_ENABLED))
			{
				c->buttons = (unsigned char*)mem_realloc(c->buttons, sizeof(unsigned char) * c->num_buttons);
				c->axes = (float*)mem_realloc(c->axes, sizeof(float) * c->num_axes);

				for (j = 0; j < c->num_buttons; j++)
					c->buttons[j] = INPUT_STATE_INIT;

				for (j = 0; j < c->num_axes; j++)
					c->axes[j] = 0.f;

				c->flags |= INPUT_FLAG_ENABLED;
			}
			
			// record button states
			for (j = 0; j < c->num_buttons; j++)
			{
				if (buttons[j])
				{
					if (c->buttons[j] & INPUT_STATE_DOWN)
						c->buttons[j] = INPUT_STATE_DOWN;
					else
						c->buttons[j] = INPUT_STATE_DOWN | INPUT_STATE_CHANGED;
				} else
				{
					if (c->buttons[j] & INPUT_STATE_DOWN)
						c->buttons[j] = INPUT_STATE_CHANGED;
					else
						c->buttons[j] = INPUT_STATE_INIT;
				}
			}

			// copy over axis values
			for (j = 0; j < c->num_axes; j++)
				c->axes[j] = axes[j];

			// left stick deadzone
			if ((c->axes[INPUT_AXIS_LEFT_LR]*c->axes[INPUT_AXIS_LEFT_LR] + c->axes[INPUT_AXIS_LEFT_UD]*c->axes[INPUT_AXIS_LEFT_UD]) < INPUT_ANALOG_DEADZONE)
			{
				c->axes[INPUT_AXIS_LEFT_LR] = 0.f;
				c->axes[INPUT_AXIS_LEFT_UD] = 0.f;
			}

			// right stick deadzone
			if ((c->axes[INPUT_AXIS_RIGHT_LR]*c->axes[INPUT_AXIS_RIGHT_LR] + c->axes[INPUT_AXIS_RIGHT_UD]*c->axes[INPUT_AXIS_RIGHT_UD]) < INPUT_ANALOG_DEADZONE)
			{
				c->axes[INPUT_AXIS_RIGHT_LR] = 0.f;
				c->axes[INPUT_AXIS_RIGHT_UD] = 0.f;
			}
		} else
		{
			// if controller was disconnected, deallocate button and axes states
			if (c->flags & INPUT_FLAG_ENABLED)
			{
				mem_free(c->buttons);
				mem_free(c->axes);

				c->flags &= ~INPUT_FLAG_ENABLED;
			}
		}
	}

	c = &im->keyboard;

	c->axes[INPUT_AXIS_LEFT_LR] = (glfwGetKey(im->win->w, GLFW_KEY_A) == GLFW_PRESS ? -1.f : 0.f) + (glfwGetKey(im->win->w, GLFW_KEY_D) == GLFW_PRESS ? 1.f : 0.f);
	c->axes[INPUT_AXIS_TRIGGERS] = (glfwGetKey(im->win->w, GLFW_KEY_W) == GLFW_PRESS ? -1.f : 0.f) + (glfwGetKey(im->win->w, GLFW_KEY_S) == GLFW_PRESS ? 1.f : 0.f);
}


const char* inputmanager_joystickname(struct inputmanager* im, int id_joyst)
{
	(void)im;
	return glfwGetJoystickName(GLFW_JOYSTICK_1 + id_joyst);
}