#include "engine_internal.h"
#include "engine_math.h"

typedef struct WindowState
{
    GLFWwindow* Handle;
    u32 Width, Height;
} WindowState;
WindowState windowState;

#define _INPUT_KEY_COUNT 1024
#define _INPUT_MOUSE_BUTTON_COUNT 128
typedef struct InputState
{
    uint8_t KeysDown[_INPUT_KEY_COUNT];
    uint8_t KeysJustDown[_INPUT_KEY_COUNT];
    uint8_t KeysReleased[_INPUT_KEY_COUNT];
    Vec2 MousePos;
    uint8_t MouseButtonDown[_INPUT_MOUSE_BUTTON_COUNT];
    uint8_t MouseButtonJustDown[_INPUT_MOUSE_BUTTON_COUNT];
    uint8_t MouseButtonReleased[_INPUT_MOUSE_BUTTON_COUNT];
} InputState;
InputState inputState;

// Callbacks
void _WindowErrorCallback(int code, const char* error_message) {    
    printf("[ERROR GLFW] %d %s\n", code, error_message);
}

void WindowSizeCallback(GLFWwindow* handle, int width, int height)
{
	windowState.Width = width;
	windowState.Height = height;
	glViewport(0, 0, width, height);
}

void _InputKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (action)
    {
    case GLFW_PRESS:    
        if (!inputState.KeysDown[key]) {
            inputState.KeysJustDown[key] = true;
        }
        inputState.KeysDown[key] = true;
        break;
    case GLFW_RELEASE:
        inputState.KeysDown[key] = false;
        inputState.KeysReleased[key] = true;
        break;
    default:
        break;
    }
}

void _InputMouseCallback(GLFWwindow* window, double x, double y)
{
    inputState.MousePos.x = x;
    inputState.MousePos.y = y;
}

void _InputMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    switch (action)
    {
    case GLFW_PRESS:
        if (!inputState.MouseButtonDown[button]) {
            inputState.MouseButtonJustDown[button] = true;
        }
        inputState.MouseButtonDown[button] = true;
        break;
    case GLFW_RELEASE:
        inputState.MouseButtonDown[button] = false;
        inputState.MouseButtonReleased[button] = true;
        break;
    default:
        break;
    }
}

// Window
boolean WindowCreate(const char* title, u32 width, u32 height)
{
    windowState.Width = width;
    windowState.Height = height;

    glfwSetErrorCallback(_WindowErrorCallback);
    if (!glfwInit()) {
        return false;
    }
    
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    windowState.Handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!windowState.Handle) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(windowState.Handle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to load glad!\n");
        return false;
    }
    
    // input callbacks
    glfwSetKeyCallback(windowState.Handle, _InputKeyCallback);
    glfwSetCursorPosCallback(windowState.Handle, _InputMouseCallback);
    glfwSetWindowSizeCallback(windowState.Handle, WindowSizeCallback);
    glfwSetMouseButtonCallback(windowState.Handle, _InputMouseButtonCallback);
    //glfwSetScrollCallback(windowState.Handle, _InputSc);

   // windowState.cursor_arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
   // windowState.cursor_hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    //GL_CALL(glDisable(GL_DEPTH_TEST));
    //GL_CALL(glClearColor(0.0, 0.2, 0.8, 0.0));
    //GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    //GL_CALL(glEnable(GL_BLEND));
    
    return true;
}

void WindowPollEvents()
{
    glfwPollEvents();
}

void WindowSwapBuffers()
{
    glfwSwapBuffers(windowState.Handle);
}

boolean WindowShouldClose()
{
    return glfwWindowShouldClose(windowState.Handle);
}

void WindowDestroy()
{   
    glfwDestroyWindow(windowState.Handle);
    //glfwDestroyCursor(window.cursor_arrow);
    //glfwDestroyCursor(window.cursor_hand);
    glfwTerminate();
}

void WindowClear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

u32 WindowWidth()
{
    return windowState.Width;
}

u32 WindowHeight()
{
    return windowState.Height;
}

// Input
boolean KeyPressed(uint32_t key)
{
    assert(key < _INPUT_KEY_COUNT);

    return inputState.KeysJustDown[key];
}

boolean KeyDown(uint32_t key)
{
    assert(key < _INPUT_KEY_COUNT);

    return inputState.KeysDown[key];
}

boolean KeyReleased(uint32_t key)
{
    assert(key < _INPUT_KEY_COUNT);

    return inputState.KeysReleased[key];
}

Vec2 MousePosition(void)
{
    return inputState.MousePos;
}

boolean MouseButtonPressed(u32 button)
{
    assert(button < _INPUT_MOUSE_BUTTON_COUNT);

    return inputState.MouseButtonJustDown[button];
}

boolean MouseButtonDown(u32 button)
{
    assert(button < _INPUT_MOUSE_BUTTON_COUNT);

    return inputState.MouseButtonDown[button];
}

boolean MouseButtonReleased(u32 button)
{
    assert(button < _INPUT_MOUSE_BUTTON_COUNT);

    return inputState.MouseButtonReleased[button];
}

void InputClear(void)
{
    for (int i = 0; i < _INPUT_KEY_COUNT; i++)
    {
        inputState.KeysJustDown[i] = false;
        inputState.KeysReleased[i] = false;
    }
    
    for (int i = 0; i < _INPUT_MOUSE_BUTTON_COUNT; i++) {
        inputState.MouseButtonJustDown[i] = false;
        inputState.MouseButtonReleased[i] = false;
    }
}
