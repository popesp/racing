#ifndef	WINDOW
#define	WINDOW

#define	_USE_MATH_DEFINES		// M_PI
#include	<math.h>			// tanf
#include	"../math/mat4f.h"	// frustum


#define	WINDOW_DEFAULT_VIEWANGLE	90.f
#define	WINDOW_DEFAULT_NEAR			0.01f
#define	WINDOW_DEFAULT_FAR			1000.f

#define	WINDOW_FLAG_NONE			0x00


struct GLFWwindow;


struct window
{
	struct GLFWwindow* w;

	int width, height;

	float near, far;
	float ratio;

	mat4f projection;

	unsigned char flags;

	char PADDING[3];
};


/*	set the viewing angle for a window
	param:	window			window of which to change viewing angle (modified) (assumed near clipping distance is defined)
	param:	angle			angle in degrees (assumed in range [0, 180])
*/
static void window_viewangle(struct window* window, float angle)
{
	window->ratio = window->near * tanf((float)M_PI * angle / 360.f);
}

/*	update the projection matrix for a window
	param:	window			window object to update (modified)
*/
static void window_updateprojection(struct window* window)
{
	float min;

	if (window->width < window->height)
	{
		min = (float)window->width / (float)window->height;
		mat4f_frustum(window->projection, -window->ratio*min, window->ratio*min, -window->ratio, window->ratio, window->near, window->far);
	}
	else
	{
		min = (float)window->height / (float)window->width;
		mat4f_frustum(window->projection, -window->ratio, window->ratio, -window->ratio*min, window->ratio*min, window->near, window->far);
	}
}

/*	resize a window
	param:	window			window object to resize (modified)
	param:	width			new width
	param:	height			new height
*/
static void window_resize(struct window* window, int width, int height)
{
	window->width = width;
	window->height = height;

	window_updateprojection(window);
}


/*	initialize window with default settings
	param:	window			window object to initialize (modified)
	param:	width			window width
	param:	height			window height
*/
static void window_init(struct window* window, int width, int height, unsigned char flags)
{
	window->near = WINDOW_DEFAULT_NEAR;
	window->far = WINDOW_DEFAULT_FAR;

	window->flags = flags;

	window_viewangle(window, WINDOW_DEFAULT_VIEWANGLE);

	window_resize(window, width, height);
}


#endif