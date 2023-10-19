void InitImGui(HWND window)
{
    // Set up Dear imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic(); // StyleColorsDark StyleColorsLight StyleColorsClassic

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, deviceContext);

}

void RenderImGui()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Toolbox", 0, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse); // Create a window called "Hello, world!" and append into it.

    ImGui::SetWindowSize(ImVec2(gFixWidth, gCurrentWindowHeight));
    ImGui::SetWindowPos(ImVec2(0, 0));
    
    ImGui::Text("%.1f FPS", io.Framerate);

    ImGui::Text("Editor mode:");               
    ImGui::RadioButton("Select Entity", &(i32)gCurrentEditorMode, EDITOR_MODE_SELECT_POLY);
    ImGui::RadioButton("Add Entity", &(i32)gCurrentEditorMode, EDITOR_MODE_ADD_POLY);
    ImGui::RadioButton("Modify Entity", &(i32)gCurrentEditorMode, EDITOR_MODE_MODIFY_POLY);
    ImGui::RadioButton("Move 3D Camera", &(i32)gCurrentEditorMode, EDITOR_MODE_MOVE_3D_CAMERA);
    ImGui::RadioButton("Set Texture", &(i32)gCurrentEditorMode, EDITOR_MODE_SET_TEXTURE);
    ImGui::RadioButton("Clipping", &(i32)gCurrentEditorMode, EDITOR_MODE_CLIPPING);

    ImGui::Text("Textures:");

    for(i32 i = 0; i < gTextureArray.size; ++i)
    {
        ImTextureID textureId = gTextureArray.guiSrv[i];
        Texture *texture = gTextureArray.cpuTextureArray + i;
        f32 uMin = 0;
        f32 vMin = 0;

        f32 uMax = 1;
        f32 vMax = 1;

        ImVec2 uvMin = ImVec2(uMin, vMin);
        ImVec2 uvMax = ImVec2(uMax, vMax);

        ImVec4 tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 backCol = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        
        ImGui::PushID(i);
        if(ImGui::ImageButton(textureId, ImVec2(170, 170), uvMin, uvMax, 1, backCol, tintCol))
        {
            gCurrentTexture = i; 
        }
        ImGui::PopID();
    }

    if(ImGui::Button("Save File"))
    {
        CreateBinaryFile_HMAP(); 
    }
    if(ImGui::Button("Load File"))
    {
        LoadBinaryFile_HMAP("../maps/test.map");
    }

    ImGui::End();
    ImGui::Render();
}

void PresentImGui()
{
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ShutDownImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
