enum MouseButonType
{
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT
};

struct MouseButton
{
    bool down;
    bool wasDown;
};

struct Input
{
    i32 x;
    i32 y;
    i32 wheelDelta;
    MouseButton buttons[3];
};

static Input gInput;
static Input gLastInput;
void InputPrepareForFrame()
{
    gInput.wheelDelta = 0;
    gInput.buttons[MOUSE_BUTTON_LEFT].wasDown = false;
    gInput.buttons[MOUSE_BUTTON_MIDDLE].wasDown = false;
    gInput.buttons[MOUSE_BUTTON_RIGHT].wasDown = false;
    if(gInput.buttons[MOUSE_BUTTON_LEFT].down) gInput.buttons[MOUSE_BUTTON_LEFT].wasDown = true;
    if(gInput.buttons[MOUSE_BUTTON_MIDDLE].down) gInput.buttons[MOUSE_BUTTON_MIDDLE].wasDown = true;
    if(gInput.buttons[MOUSE_BUTTON_RIGHT].down) gInput.buttons[MOUSE_BUTTON_RIGHT].wasDown = true;
}

i32 MouseX()
{
    return gInput.x;
}

i32 MouseY()
{
    return gInput.y;
}

i32 MouseLastX()
{
    return gLastInput.x;
}

i32 MouseLastY()
{
    return gLastInput.y;
}

i32 MouseWheelDelta()
{
    return gInput.wheelDelta;
}

bool MouseIsDown(i32 mouseButton)
{
    return gInput.buttons[mouseButton].down;
}

bool MouseJustDown(i32 mouseButton)
{
    if(gInput.buttons[mouseButton].down != gInput.buttons[mouseButton].wasDown)
    {
        if(gInput.buttons[mouseButton].down)
        {
            return true;
        }
    }
    return false;
}


bool MouseJustUp(i32 mouseButton)
{
    if(gInput.buttons[mouseButton].down != gInput.buttons[mouseButton].wasDown)
    {
        if(gInput.buttons[mouseButton].wasDown)
        {
            return true;
        }
    }
    return false;
}
