#include "Text.hpp"

Text::Text()
{
    m_Font = nullptr;
    m_screenWidth = 0;
    m_screenHeight = 0;
}

Text::~Text()
{

}

bool Text::Initialize(ID3D11Device* device, Font* font, int screenWidth, int screenHeight)
{
    m_Font = font;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    return true;
}

void Text::Shutdown()
{
    for (auto& sentence : m_Sentences)
    {
        ShutdownSentence(sentence);
    }

    m_Sentences.clear();

    m_Font = nullptr;
}

int Text::AddSentence(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& text, int maxLength, int positionX, int positionY, float red, float green, float blue)
{
    SentenceType sentence;
    bool result;

    sentence.vertexBuffer = 0;
    sentence.indexBuffer = 0;
    sentence.maxLength = maxLength;
    sentence.vertexCount = 0;
    sentence.indexCount = 0;
    sentence.positionX = positionX;
    sentence.positionY = positionY;
    sentence.red = red;
    sentence.green = green;
    sentence.blue = blue;

    result = InitializeSentenceBuffers(device, sentence, maxLength);
    if (!result)
    {
        return -1;
    }

    result = UpdateSentenceBuffers(deviceContext, sentence, text);
    if (!result)
    {
        ShutdownSentence(sentence);
        return -1;
    }

    m_Sentences.push_back(sentence);

    return (int)(m_Sentences.size() - 1);
}

bool Text::UpdateSentence(ID3D11DeviceContext* deviceContext, int sentenceIndex, const std::string& text)
{
    if (sentenceIndex < 0 || sentenceIndex >= (int)m_Sentences.size())
    {
        return false;
    }

    return UpdateSentenceBuffers(deviceContext, m_Sentences[sentenceIndex], text);
}

bool Text::InitializeSentenceBuffers(ID3D11Device* device, SentenceType& sentence, int maxLength)
{
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA indexData;
    HRESULT result;
    std::vector<unsigned long> indices;
    int maxVertices, i;

    maxVertices = 6 * maxLength;

    // DYNAMIC, no initial data - identical reasoning to BitmapClass: this
     // buffer's actual contents get written later (and rewritten whenever
     // the text changes) via UpdateSentenceBuffers, not once at creation.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(Font::VertexType) * maxVertices;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&vertexBufferDesc, NULL, &sentence.vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // The index buffer only ever needs to be 0,1,2,3...maxVertices-1 in
    // order - it never changes even as the text itself does, since every
    // draw call just tells DrawIndexed how many of those indices to
    // actually use (sentence.indexCount, updated per text change). So
    // unlike the vertex buffer, this one is a normal static USAGE_DEFAULT
    // buffer, built once here and never touched again.
    indices.resize(maxVertices);
    for (i = 0; i < maxVertices; i++)
    {
        indices[i] = (unsigned long)i;
    }

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * maxVertices;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    indexData.pSysMem = indices.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    result = device->CreateBuffer(&indexBufferDesc, &indexData, &sentence.indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool Text::UpdateSentenceBuffers(ID3D11DeviceContext* deviceContext, SentenceType& sentence, const std::string& text)
{
    int numLetters;
    float drawX, drawY;
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    numLetters = (int)text.length();
    if (numLetters > sentence.maxLength)
    {
        // Caller asked for more text than this sentence reserved room for
        // at AddSentence time - refuse rather than silently truncating or
        // overrunning the buffer.
        return false;
    }

    sentence.vertexCount = 6 * numLetters;
    sentence.indexCount = sentence.vertexCount;

    if (numLetters == 0)
    {
        return true;
    }

    std::vector<Font::VertexType> vertices(sentence.vertexCount);

    // Same screen-pixel -> ortho-space conversion as BitmapClass/SpriteClass.
    drawX = (float)(((m_screenWidth / 2) * -1) + sentence.positionX);
    drawY = (float)((m_screenHeight / 2) - sentence.positionY);

    m_Font->BuildVertexArray(vertices.data(), text.c_str(), drawX, drawY);

    result = deviceContext->Map(sentence.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Only the bytes for THIS text's actual vertex count get written -
    // whatever's left over in the (larger, maxLength-sized) buffer past
    // that point is simply never referenced, since the draw call in
    // Render() below uses sentence.indexCount, not the buffer's full size.
    memcpy(mappedResource.pData, vertices.data(), sizeof(Font::VertexType) * sentence.vertexCount);

    deviceContext->Unmap(sentence.vertexBuffer, 0);

    return true;
}


void Text::ShutdownSentence(SentenceType& sentence)
{
    if (sentence.vertexBuffer)
    {
        sentence.vertexBuffer->Release();
        sentence.vertexBuffer = 0;
    }

    if (sentence.indexBuffer)
    {
        sentence.indexBuffer->Release();
        sentence.indexBuffer = 0;
    }
}

bool Text::Render(ID3D11DeviceContext* deviceContext, FontShader* fontShader, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX orthoMatrix)
{
    unsigned int stride, offset;
    XMFLOAT4 pixelColor;
    bool result;

    stride = sizeof(Font::VertexType);
    offset = 0;

    for (auto& sentence : m_Sentences)
    {
        if (sentence.indexCount == 0)
        {
            continue;
        }

        deviceContext->IASetVertexBuffers(0, 1, &sentence.vertexBuffer, &stride, &offset);
        deviceContext->IASetIndexBuffer(sentence.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        pixelColor = XMFLOAT4(sentence.red, sentence.green, sentence.blue, 1.0f);

        result = fontShader->Render(deviceContext, sentence.indexCount, worldMatrix, viewMatrix, orthoMatrix,
            m_Font->GetTexture(), pixelColor);
        if (!result)
        {
            return false;
        }
    }

    return true;
}