struct LineVertex 
{
    Vec3 position;
    Vec4 color;
};

static ID3D11Buffer      *gGPULineBuffer;
static ID3D11InputLayout *gGPULineLayout;
static LineVertex        *gCPULineBuffer;
static size_t             gLineBufferSize;
static size_t             gLineBufferSizeInBytes;
static size_t             gLineBufferUsed;
static Shader             gLineShader;

static void LineRendererInitialize(size_t bufferSize) 
{
    gLineBufferSize = bufferSize;
    gLineBufferUsed = 0;
    gLineShader = LoadShader("../src/shaders/lineVert.hlsl", "../src/shaders/lineFrag.hlsl");

    size_t bufferSizeInBytes = gLineBufferSize*sizeof(LineVertex);
    // create gGPUBuffer
    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexDesc.ByteWidth = bufferSizeInBytes;
    HRESULT result = device->CreateBuffer(&vertexDesc, NULL, &gGPULineBuffer);

    // create gGPULayout
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    i32 totalLayoutElements = ARRAY_LENGTH(inputLayoutDesc);
    result = device->CreateInputLayout(inputLayoutDesc,
        totalLayoutElements,
        gLineShader.vertexShaderCompiled->GetBufferPointer(),
        gLineShader.vertexShaderCompiled->GetBufferSize(),
        &gGPULineLayout);

    // create gCPUBuffer
    gCPULineBuffer = (LineVertex *)malloc(bufferSizeInBytes);
    memset(gCPULineBuffer, 0, bufferSizeInBytes);
}

static void LineRendererShutdown()
{
    if(gGPULineBuffer) gGPULineBuffer->Release();
    if(gGPULineLayout) gGPULineLayout->Release();
    if(gCPULineBuffer) free(gCPULineBuffer);
    UnloadShader(&gLineShader);
}

static void LineRendererDraw()
{
    D3D11_MAPPED_SUBRESOURCE bufferData;
    ZeroMemory(&bufferData, sizeof(bufferData));
    deviceContext->Map(gGPULineBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
    memcpy(bufferData.pData, gCPULineBuffer, sizeof(LineVertex)*gLineBufferUsed);
    deviceContext->Unmap(gGPULineBuffer, 0);

    u32 stride = sizeof(LineVertex);
    u32 offset = 0;
    deviceContext->IASetInputLayout(gGPULineLayout);
    deviceContext->VSSetShader(gLineShader.vertex, 0, 0);
    deviceContext->PSSetShader(gLineShader.fragment, 0, 0);
    deviceContext->IASetVertexBuffers(0, 1, &gGPULineBuffer, &stride, &offset);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    deviceContext->Draw(gLineBufferUsed, 0);
    gLineBufferUsed = 0;
}

static void LocalToWorldLine(LineVertex *line, u32 count, f32 ax, f32 ay, f32 az, f32 bx, f32 by, f32 bz)
{
    Mat4 world = Mat4Translate(ax, ay, az);
    line[0].position = Mat4TransformPoint(world, line[0].position);
    world = Mat4Translate(bx, by, bz);
    line[1].position = Mat4TransformPoint(world, line[1].position);
}

static void SetColor(LineVertex *line, u32 count, u32 color)
{
    f32 a = (f32)((color >> 24) & 0xFF) / 255.0f;
    f32 r = (f32)((color >> 16) & 0xFF) / 255.0f;
    f32 g = (f32)((color >>  8) & 0xFF) / 255.0f;
    f32 b = (f32)((color >>  0) & 0xFF) / 255.0f;
    for(i32 i = 0; i < count; ++i) {
        line[i].color = {r, g, b, a};
    }
}

static void AddLineToBuffer(LineVertex *line, u32 count)
{ 
    ASSERT(gLineBufferUsed + count < gLineBufferSize);
    LineVertex *vertex = gCPULineBuffer + gLineBufferUsed;
    memcpy(vertex, line, sizeof(LineVertex)*count);
    gLineBufferUsed += count;
}

void DrawLine(f32 ax, f32 ay, f32 az, f32 bx, f32 by, f32 bz, u32 color)
{

    LineVertex line[2] = {};
    LocalToWorldLine(line, 2, ax, ay, az, bx, by, bz);
    SetColor(line, 2, color);

    if(gLineBufferUsed + 2 >= gLineBufferSize) {
        LineRendererDraw();
    }
    AddLineToBuffer(line, 2);

}
