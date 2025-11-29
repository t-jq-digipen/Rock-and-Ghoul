/*!
@file       glapp.cpp
@author     pghali@digipen.edu
@co-author  parminder.singh@digipen.edu
@date       05/05/2025

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include "../include/glapp.h"
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "Input.h" 
#include "Render.h"
#include "Registry.h"
#include "CoreEngine.h"
#include "GuiSystem.h"
#include <Camera.h>
// Before asking GLFW to create an OpenGL context, we specify the minimum constraints
// in that context:
#define GL_MINOR 5
#define GL_MAJOR 4

bool GLApp::gameInitialized = false;
/*                                                   objects with file scope
----------------------------------------------------------------------------- */
GLint GLApp::width = 0;
GLint GLApp::height = 0;
GLint GLApp::baseWidth = 0;
GLint GLApp::baseHeight = 0;
GLdouble GLApp::fps = 0.0;
GLdouble GLApp::delta_time = 0.0;
std::string GLApp::title;
GLFWwindow* GLApp::ptr_window = nullptr;
GLfloat GLApp::clear_color[4] = { 0.f, 0.f, 0.f, 1.f };
GLfloat GLApp::VIRTUAL_H = 1080.f;
GLfloat GLApp::VIRTUAL_W = 1920.f;



/*  _________________________________________________________________________ */
/*! init

@param GLint width
@param GLint height
Dimensions of window requested by program

@param std::string title_str
String printed to window's title bar

@return bool
true if OpenGL context and GLEW were successfully initialized.
false otherwise.

Uses GLFW to create OpenGL context. GLFW's initialization follows from here:
http://www.glfw.org/docs/latest/quick.html
a window of size width x height pixels
and its associated OpenGL context that matches a core profile that is
compatible with OpenGL 4.5 and doesn't support "old" OpenGL, has 32-bit RGBA,
double-buffered color buffer, 24-bit depth buffer and 8-bit stencil buffer
with each buffer of size width x height pixels
*/
bool GLApp::init(GLint w, GLint h, std::string t, bool enable) {
 /* GLApp::width = w;
  GLApp::height = h;
  GLApp::baseWidth = w;
  GLApp::baseHeight = h;*/
  GLApp::title = t;
  windowed_width = w;
  windowed_height = h;

   GLApp::baseWidth = w;
    GLApp::baseHeight = h;
  //is_fullscreen = false;
  if (!glfwInit()) {
    std::cout << "GLFW init has failed - abort program!!!" << std::endl;
    return false;
  }
  // In case a GLFW function fails, an error is reported to callback function
  glfwSetErrorCallback(GLApp::error_cb);

  // specify OpenGL version 4.5
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  // specify modern OpenGL only - no compatibility with "old" OpenGL
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // applications will be double-buffered ...
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  // default behavior: colorbuffer is 32-bit RGBA, depthbuffer is 24-bits
  // don't change size of window
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //Make Window non-resizable (im assuming we using 1920x1080 for it?)
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);


  // size of viewport: width x height
  //GLApp::ptr_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
  //if (!GLApp::ptr_window) {
  //  std::cerr << "GLFW unable to create OpenGL context - abort program\n";
  //  glfwTerminate();
  //  return false;
  //}
  //GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  //const GLFWvidmode* mode = glfwGetVideoMode(monitor);

  //// Create window in fullscreen mode directly
  //GLApp::ptr_window = glfwCreateWindow(
  //    mode->width,
  //    mode->height,
  //    title.c_str(),
  //    monitor,  // Pass monitor for fullscreen
  //    NULL
  //);
  GLApp::ptr_window = glfwCreateWindow(
      w,           // Use requested width
      h,           // Use requested height
      title.c_str(),
      NULL,        // NULL = windowed mode
      NULL
  );
  if (!GLApp::ptr_window) {
      std::cerr << "GLFW unable to create OpenGL context - abort program\n";
      glfwTerminate();
      return false;
  }

  //GLApp::width = mode->width;
  //GLApp::height = mode->height;

  GLApp::width = w;
  GLApp::height = h;
  GLApp::baseWidth = w;
  GLApp::baseHeight = h;
  is_fullscreen = false;
  // make the previously created OpenGL context current ...
  glfwMakeContextCurrent(GLApp::ptr_window);
  //glViewport(0, 0, mode->width, mode->height);
   glViewport(0, 0, w, h);
  // set callback for events associated with window size changes; keyboard;
 // mouse buttons, cursor position, and scroller
  glfwSetFramebufferSizeCallback(GLApp::ptr_window, GLApp::fbsize_cb);
  glfwSetKeyCallback(GLApp::ptr_window, Input::KeyCallback);
  glfwSetMouseButtonCallback(GLApp::ptr_window, Input::MouseButtonCallback);
  glfwSetCursorPosCallback(GLApp::ptr_window, Input::MousePosCallback);
  glfwSetScrollCallback(GLApp::ptr_window, Input::MouseScrollCallback);

  // this is the default setting ...
  glfwSetInputMode(GLApp::ptr_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // initialize OpenGL (and extension) function loading library
  GLenum err = glewInit();

  // Start in fullscreen if enabled
  if (enable) {
      GLApp::SetFullscreenMode();
  }

  glfwShowWindow(ptr_window);
  glfwFocusWindow(ptr_window);

  if (GLEW_OK != err) {
    std::cerr << "Unable to initialize GLEW - error: "
      << glewGetErrorString(err) << " abort program" << std::endl;
    return false;
  }
  std::cout << "Using glew version: " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

  const GLubyte* glVersion = glGetString(GL_VERSION);
  std::cout << "OpenGL Version String: " << glVersion << std::endl;

  int major, minor;
  int scanresult = sscanf(reinterpret_cast<const char*>(glVersion), "%d.%d", &major, &minor);
  if (scanresult != 2) {
      return false;
  }
  if (major == GL_MAJOR && minor == GL_MINOR) {
      std::cout << "OpenGL 4.5 is supported." << std::endl;
  }
  else {
      std::cerr << "OpenGL 4.5 is not supported." << std::endl;
      return false;
  }

  // we'll use the entire window as viewport ...
  glm::ivec2 vp_width_height(0, 0);
  glfwGetFramebufferSize(ptr_window, &vp_width_height.x, &vp_width_height.y);
  GLApp::fbsize_cb(ptr_window, vp_width_height.x, vp_width_height.y);

  return true;
}

/*  _________________________________________________________________________ */
/*! update

@param none

@return none

For now, there is nothing to do except to write appropriate information
to window title bar.
*/
void GLApp::update() {
  // write window title with current fps ...
  std::stringstream sstr;
  sstr << std::fixed << std::setprecision(2) << GLApp::title << " | " << GLApp::fps;
  glfwSetWindowTitle(GLApp::ptr_window, sstr.str().c_str());


}

/*  _________________________________________________________________________ */
/*! draw

@param none

@return none

For now, there is nothing to do except set the back buffer fill color
as RGB(0, 1, 0).
*/
void GLApp::draw() {
  // clear colorbuffer with RGBA value in glClearColor ...
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
}

/*  _________________________________________________________________________ */
/*! cleanup

@param none

@return none

For now, there are no resources allocated by the application program.
The only task is to have GLFW return resources back to the system and
gracefully terminate.
*/
void GLApp::cleanup() {
  glfwTerminate();
}


/*  _________________________________________________________________________*/
/*! key_cb

@param GLFWwindow*
Handle to window that is receiving event

@param int
the keyboard key that was pressed or released

@parm int
Platform-specific scancode of the key

@parm int
GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
action will be GLFW_KEY_UNKNOWN if GLFW lacks a key token for it,
for example E-mail and Play keys.

@parm int
bit-field describing which modifier keys (shift, alt, control)
were held down

@return none

This function is called when keyboard buttons are pressed.
When the ESC key is pressed, the close flag of the window is set.
*/
void GLApp::key_cb(GLFWwindow *pwin, int key, int scancode, int action, int mod) {
    (void)scancode;
    (void)mod;
 if (GLFW_PRESS == action) {
#ifdef _DEBUG
    std::cout << "Key pressed" << std::endl;
#endif
  } else if (GLFW_REPEAT == action) {
#ifdef _DEBUG
    std::cout << "Key repeatedly pressed" << std::endl;
#endif
  } else if (GLFW_RELEASE == action) {
#ifdef _DEBUG
    std::cout << "Key released" << std::endl;
#endif
  }

  if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
    glfwSetWindowShouldClose(pwin, GLFW_TRUE);
  }

  //if (key == GLFW_KEY_ENTER && action == GLFW_PRESS &&
  //    (mod & GLFW_MOD_ALT))
  //{
  //    ToggleFullscreen();
  //    return;
  //}
}

/*  _________________________________________________________________________*/
/*! mousebutton_cb

@param GLFWwindow*
Handle to window that is receiving event

@param int
the mouse button that was pressed or released
GLFW_MOUSE_BUTTON_LEFT and GLFW_MOUSE_BUTTON_RIGHT specifying left and right
mouse buttons are most useful

@parm int
action is either GLFW_PRESS or GLFW_RELEASE

@parm int
bit-field describing which modifier keys (shift, alt, control)
were held down

@return none

This function is called when mouse buttons are pressed.
*/
void GLApp::mousebutton_cb(GLFWwindow *pwin, int button, int action, int mod) {
    (void)pwin;
    (void)mod;
  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
#ifdef _DEBUG
    std::cout << "Left mouse button ";
#endif
    break;
  case GLFW_MOUSE_BUTTON_RIGHT:
#ifdef _DEBUG
    std::cout << "Right mouse button ";
#endif
    break;
  }
  switch (action) {
  case GLFW_PRESS:
#ifdef _DEBUG
    std::cout << "pressed!!!" << std::endl;
#endif
    break;
  case GLFW_RELEASE:
#ifdef _DEBUG
    std::cout << "released!!!" << std::endl;
#endif
    break;
  }
}

/*  _________________________________________________________________________*/
/*! mousepos_cb

@param GLFWwindow*
Handle to window that is receiving event

@param double
new cursor x-coordinate, relative to the left edge of the client area

@param double
new cursor y-coordinate, relative to the top edge of the client area

@return none

This functions receives the cursor position, measured in screen coordinates but
relative to the top-left corner of the window client area.
*/
void GLApp::mousepos_cb(GLFWwindow *pwin, double xpos, double ypos) {
#ifdef _DEBUG
    (void)pwin;
  std::cout << "Mouse cursor position: (" << xpos << ", " << ypos << ")" << std::endl;
#endif
}

/*  _________________________________________________________________________*/
/*! mousescroll_cb

@param GLFWwindow*
Handle to window that is receiving event

@param double
Scroll offset along X-axis

@param double
Scroll offset along Y-axis

@return none

This function is called when the user scrolls, whether with a mouse wheel or
touchpad gesture. Although the function receives 2D scroll offsets, a simple
mouse scroll wheel, being vertical, provides offsets only along the Y-axis.
*/
void GLApp::mousescroll_cb(GLFWwindow *pwin, double xoffset, double yoffset) {
#ifdef _DEBUG
    (void)pwin;
  std::cout << "Mouse scroll wheel offset: ("
    << xoffset << ", " << yoffset << ")" << std::endl;
#endif
}

/*  _________________________________________________________________________ */
/*! error_cb

@param int
GLFW error code

@parm char const*
Human-readable description of the code

@return none

The error callback receives a human-readable description of the error and
(when possible) its cause.
*/
void GLApp::error_cb(int error, char const* description) {
    (void)error;
  std::cerr << "GLFW error: " << description << std::endl;
}

/*  _________________________________________________________________________ */
/*! fbsize_cb

@param GLFWwindow*
Handle to window that is being resized

@parm int
Width in pixels of new window size

@parm int
Height in pixels of new window size

@return none

This function is called when the window is resized - it receives the new size
of the window in pixels.
*/
void GLApp::fbsize_cb(GLFWwindow *ptr_win, int newwidth, int newheight) {
  // use the entire framebuffer as drawing region
    (void)ptr_win;
    //scale factors BEFORE updating GLApp::width/height
    float scaleX = (float)newwidth / (float)GLApp::baseWidth;
    float scaleY = (float)newheight / (float)GLApp::baseHeight;


    //update the global width/height
    GLApp::width = newwidth;
    GLApp::height = newheight;

    glViewport(0, 0, newwidth, newheight);
    Render::GetInstance().InitSceneFramebuffer(newwidth, newheight);
    Render::GetInstance().SetWindowScale(scaleX, scaleY);

    //Camera::GetInstance().UpdateMatrices();     // updates orthographic projection
    //Render::GetInstance().UpdateProjection();   // send projection to sprite shader
    //if (GLApp::gameInitialized) {
    //    Registry& reg = Registry::Get();

    //    // Scale all entity positions
    //    for (auto& [entID, transform] : reg.Get_ComponentMap<TransformComponent>()) {
    //        transform.pos.x *= scaleX;
    //        transform.pos.y *= scaleY;
    //        transform.scale.x *= scaleX;
    //        transform.scale.y *= scaleY;
    //    }

    //    // update AABBs
    //    for (auto& [entID, physics] : reg.Get_ComponentMap<Physics::PhysicsComponent>()) {
    //        if (reg.Has_Component<TransformComponent>(entID)) {
    //            auto& transform = reg.Get_Component<TransformComponent>(entID);
    //            // aabb
    //            AABB& aabb = physics.Get_AABB();
    //            aabb.min.x *= scaleX;
    //            aabb.max.x *= scaleX;
    //            aabb.min.y *= scaleY;
    //            aabb.max.y *= scaleY;
    //            aabb.dim.x *= scaleX;
    //            aabb.dim.y *= scaleY;
    //            //circle
    //            Circle& circle = physics.Get_Circle();
    //            circle.pos.x = transform.pos.x;
    //            circle.pos.y = transform.pos.y;
    //            circle.rad *= (scaleX + scaleY) * 0.5f;

    //            
    //            physics.UpdateHitboxes(transform.pos);
    //        }
    //    }
    //    // scale all sprite components
    //    for (auto& [entID, sprite] : reg.Get_ComponentMap<Sprite::SpriteComponent>()) {
    //        Vec2 currentScale = sprite.Get_Scale();
    //        sprite.Set_Size({
    //            currentScale.x * scaleX,
    //            currentScale.y * scaleY
    //            });
    //    }

    //    for (auto& [entID, shape] : reg.Get_ComponentMap<ShapeComponent>()) {
    //        shape.dimensions.x *= scaleX;
    //        shape.dimensions.y *= scaleY;
    //    }
    //}
}

/*  _________________________________________________________________________*/
/*! update_time

@param double
fps: computed frames per second

@param double
fps_calc_interval: the interval (in seconds) at which fps is to be
calculated

@return none

This function must be first called once during initialization and once
per game loop. It uses GLFW's time functions to compute:
1. interval in seconds between each frame
2. frames per second every "fps_calc_interval" seconds
*/
void GLApp::update_time(double fps_calc_interval) {
  // get elapsed time (in seconds) between previous and current frames
  static double prev_time = glfwGetTime();
  double curr_time = glfwGetTime();
  GLApp::delta_time = curr_time - prev_time;
  prev_time = curr_time;

  // fps calculations
  static double count = 0.0; // number of game loop iterations
  static double start_time = glfwGetTime();
  // get elapsed time since very beginning (in seconds) ...
  double elapsed_time = curr_time - start_time;

  ++count;

  // update fps at least every 10 seconds ...
  fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
  fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
  if (elapsed_time > fps_calc_interval) {
    GLApp::fps = count / elapsed_time;
    start_time = curr_time;
    count = 0.0;
  }
}

// Add to file scope variables section
bool GLApp::is_fullscreen = false;
int GLApp::windowed_width = 1920;
int GLApp::windowed_height = 1080;
int GLApp::windowed_pos_x = 100;
int GLApp::windowed_pos_y = 100;

/*  _________________________________________________________________________ */
/*! ToggleFullscreen

@return none

Switches between fullscreen and windowed mode.
Saves windowed position/size when going fullscreen.
Restores windowed position/size when exiting fullscreen.
*/
void GLApp::ToggleFullscreen()
{
    if (!ptr_window) return;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (is_fullscreen)
    {
        // switch to window
        std::cout << "[GLApp] Switching to Windowed mode (" << windowed_width << "x" << windowed_height << ")\n";

        glfwSetWindowMonitor(ptr_window, nullptr, windowed_pos_x, windowed_pos_y, windowed_width, windowed_height, GLFW_DONT_CARE);
        GLApp::width = windowed_width;
        GLApp::height = windowed_height;
        is_fullscreen = false;
    }
    else
    {
        // save current window position and size before going fullscreen
        glfwGetWindowPos(ptr_window, &windowed_pos_x, &windowed_pos_y);
        glfwGetWindowSize(ptr_window, &windowed_width, &windowed_height);

        // switch to Fullscreen
        std::cout << "[GLApp] Switching to Fullscreen mode ("  << mode->width << "x" << mode->height << ")\n";

        glfwSetWindowMonitor(ptr_window, monitor,  0, 0, mode->width, mode->height, mode->refreshRate);
        GLApp::width = mode->width;
        GLApp::height = mode->height;
        is_fullscreen = true;
    }

    //framebuffer size callback to update viewport
    int newWidth, newHeight;
    glfwGetFramebufferSize(ptr_window, &newWidth, &newHeight);
    fbsize_cb(ptr_window, newWidth, newHeight);
}

/*  _________________________________________________________________________ */
/*! SetWindowedMode

@param width - desired window width in pixels
@param height - desired window height in pixels

@return none

Sets the window to windowed mode with specified dimensions.
*/
void GLApp::SetWindowedMode(int p_width, int p_height)
{
    if (!ptr_window) return;

    if (is_fullscreen)
    {
        // exit fullscreen first
        ToggleFullscreen();
    }

    windowed_width = p_width;
    windowed_height = p_height;

    std::cout << "[GLApp] Setting windowed resolution: "  << p_width << "x" << p_height << "\n";

    glfwSetWindowSize(ptr_window, p_width, p_height);

    // center window on screen
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    windowed_pos_x = (mode->width - p_width) / 2;
    windowed_pos_y = (mode->height - p_height) / 2;
    glfwSetWindowPos(ptr_window, windowed_pos_x, windowed_pos_y);

    // update viewport
    int newWidth, newHeight;
    glfwGetFramebufferSize(ptr_window, &newWidth, &newHeight);
    fbsize_cb(ptr_window, newWidth, newHeight);
}

/*  _________________________________________________________________________ */
/*! SetFullscreenMode

@return none

Sets the window to fullscreen mode using primary monitor's resolution.
*/
void GLApp::SetFullscreenMode()
{
    if (!ptr_window || is_fullscreen) return;

    ToggleFullscreen();
}

/*  _________________________________________________________________________ */
/*! IsFullscreen

@return bool - true if window is in fullscreen mode, false otherwise
*/
bool GLApp::IsFullscreen()
{
    return is_fullscreen;
}