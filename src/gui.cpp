static bool gShowTexturesWindow = false;
static bool gShowErrorWindow = false;
static char *gDefaultSavePath = "../maps/test2.map";

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

void ErrorWindow() {
    ImGui::Begin("Error", &gShowErrorWindow, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize(ImVec2(200, 100));
    ImGui::SetWindowPos(ImVec2((gCurrentWindowWidth/2)-100, (gCurrentWindowHeight/2)-50));
    ImGui::Text("ERROR");
    ImGui::End();
}

void RenderImGui()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // if(gShowTexturesWindow) ImGui::ShowDemoWindow(&gShowTexturesWindow);

    ImGui::Begin("Toolbox", 0, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse);

    ImGui::SetWindowSize(ImVec2(gFixWidth, gCurrentWindowHeight));
    ImGui::SetWindowPos(ImVec2(0, 0));

    ImGui::SeparatorText("Save File:");
    static char saveToPath[MAX_PATH] = "";
    ImGui::InputTextWithHint("path 0", "save to path", saveToPath, IM_ARRAYSIZE(saveToPath));
    if(ImGui::Button("Save as"))
    {
        if(strcmp(saveToPath, "") != 0) {
            gShowErrorWindow = !CreateBinaryFile_HMAP(saveToPath); 
        }
    }
 
    ImGui::SeparatorText("Open file:");
    static char openFromPath[MAX_PATH] = "";
    ImGui::InputTextWithHint("path 1", "open from path", openFromPath, IM_ARRAYSIZE(openFromPath));
    if(ImGui::Button("Open"))
    {
        // TODO: check if the path is valide before removing the old textures
        if(strcmp(openFromPath, "") != 0) {
            UnloadTextureArray(&gTextureArray);
            gShowErrorWindow = !LoadBinaryFile_HMAP(openFromPath);
        }
    }
    
    ImGui::SeparatorText("App Info:");
    ImGui::Text("%.1f FPS", io.Framerate);
    ImGui::Text("%lfgb / %lfgb",
                (f64)gDynamicVertexBuffer.used / (f64)GIGABYTES(1),
                (f64)gDynamicVertexBuffer.size / (f64)GIGABYTES(1));

    ImGui::SeparatorText("Editor mode:");
    ImGui::RadioButton("Select Entity", &(i32)gCurrentEditorMode, EDITOR_MODE_SELECT_POLY);
    ImGui::RadioButton("Add Entity", &(i32)gCurrentEditorMode, EDITOR_MODE_ADD_POLY);
    ImGui::RadioButton("Modify Entity", &(i32)gCurrentEditorMode, EDITOR_MODE_MODIFY_POLY);
    ImGui::RadioButton("Move 3D Camera", &(i32)gCurrentEditorMode, EDITOR_MODE_MOVE_3D_CAMERA);
    ImGui::RadioButton("Set Texture", &(i32)gCurrentEditorMode, EDITOR_MODE_SET_TEXTURE);
    ImGui::RadioButton("Clipping", &(i32)gCurrentEditorMode, EDITOR_MODE_CLIPPING);

    ImGui::SeparatorText("Textures");
    ImGui::Text("the textures must be");               
    ImGui::Text("at least 256x256 px");               
    static char texturePath[MAX_PATH] = "";
    ImGui::InputTextWithHint("path 2", "texture path", texturePath, IM_ARRAYSIZE(texturePath));
    if(ImGui::Button("Load Texture"))
    {
        if(strcmp(texturePath, "") != 0) {
            gShowErrorWindow = !LoadTextureToTextureArray(&gTextureArray, texturePath);
        }
    }
    if(ImGui::Button("Show Textures"))
    {
        gShowTexturesWindow = !gShowTexturesWindow; 
    }

    ImGui::End();

    if(gShowTexturesWindow) {
        ImGui::Begin("Textures", &gShowTexturesWindow, ImGuiWindowFlags_NoCollapse);

        ImGuiStyle& style = ImGui::GetStyle();
        i32 buttons_count = gTextureArray.size;
        f32 window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        for (i32 i = 0; i < buttons_count; i++)
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
            if(ImGui::ImageButton(textureId, ImVec2(128, 128), uvMin, uvMax, 1, backCol, tintCol))
            {
                gCurrentTexture = i; 
            }

            f32 max = ImGui::GetItemRectMax().x;
            f32 min = ImGui::GetItemRectMin().x;

            f32 last_button_x2 = ImGui::GetItemRectMax().x;
            f32 next_button_x2 = last_button_x2 + style.ItemSpacing.x + (max - min);
            
            if (i + 1 < buttons_count && next_button_x2 < window_visible_x2)
                ImGui::SameLine();


            ImGui::PopID();
        }

        ImGui::End();
    }


    if(gShowErrorWindow) {
        ErrorWindow();
    }

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
