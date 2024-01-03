enum MouseButonType
{
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT
};

struct Button
{
    bool down;
    bool wasDown;
};

struct Input
{
    i32 x;
    i32 y;
    i32 wheelDelta;
    Button buttons[3];
    Button keys[349];

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

    for(i32 i = 0; i < ARRAY_LENGTH(gInput.keys); ++i)
    {
        gInput.keys[i].wasDown = false;
        if(gInput.keys[i].down) gInput.keys[i].wasDown = true;
    }
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(io.WantCaptureMouse) return false;

    return gInput.wheelDelta;
}

bool MouseIsDown(i32 mouseButton)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(io.WantCaptureMouse) return false;

    return gInput.buttons[mouseButton].down;
}

bool MouseJustDown(i32 mouseButton)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(io.WantCaptureMouse) return false;

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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(io.WantCaptureMouse) return false;

    if(gInput.buttons[mouseButton].down != gInput.buttons[mouseButton].wasDown)
    {
        if(gInput.buttons[mouseButton].wasDown)
        {
            return true;
        }
    }
    return false;
}

bool KeyIsDown(i32 key)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(io.WantCaptureKeyboard) return false;

    return gInput.keys[key].down;
}

bool KeyJustDown(i32 key)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(io.WantCaptureKeyboard) return false;

    if(gInput.keys[key].down != gInput.keys[key].wasDown)
    {
        if(gInput.keys[key].down)
        {
            return true;
        }
    }
    return false;
}

bool KeyJustUp(i32 key)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(io.WantCaptureKeyboard) return false;

    if(gInput.keys[key].down != gInput.keys[key].wasDown)
    {
        if(gInput.keys[key].wasDown)
        {
            return true;
        }
    }
    return false;
}
