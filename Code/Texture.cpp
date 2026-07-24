#include "Texture.hpp"
#include "Utils.hpp"

Texture::Texture()
{
	m_imageData = 0;
	m_texture = 0;
	m_textureView = 0;
}

Texture::~Texture()
{

}

bool Texture::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename)
{
	bool result;
	HRESULT hResult;
	D3D11_TEXTURE2D_DESC textureDesc;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	result = LoadImageWIC(filename);
	if (!result)
	{
		return false;
	}

	textureDesc.Height = m_height;
	textureDesc.Width = m_width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult))
	{
		return false;
	}

	rowPitch = (m_width * 4) * sizeof(unsigned char);
	deviceContext->UpdateSubresource(m_texture, 0, NULL, m_imageData, rowPitch, 0);

	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
	{
		return false;
	}

	deviceContext->GenerateMips(m_textureView);

	delete[] m_imageData;
	m_imageData = 0;

	return true;
}

void Texture::Shutdown()
{
	if (m_textureView)
	{
		m_textureView->Release();
		m_textureView = 0;
	}

	if (m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	if (m_imageData)
	{
		delete[] m_imageData;
		m_imageData = 0;
	}
}

ID3D11ShaderResourceView* Texture::GetTexture()
{
	return m_textureView;
}

bool Texture::LoadImageWIC(const char* filename)
{
	HRESULT hResult;
	IWICImagingFactory* wicFactory = 0;
	IWICBitmapDecoder* decoder = 0;
	IWICBitmapFrameDecode* frame = 0;
	IWICFormatConverter* converter = 0;
	UINT width, height;
	UINT stride, bufferSize;

	hResult = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
	if (FAILED(hResult))
	{
		return false;
	}

	hResult = wicFactory->CreateDecoderFromFilename(Utils::ConvertToWChar(filename), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
	if (FAILED(hResult))
	{
		wicFactory->Release();
		return false;
	}

	// Most images have only one frame, we only want frame 0
	hResult = decoder->GetFrame(0, &frame);
	if (FAILED(hResult))
	{
		decoder->Release();
		wicFactory->Release();
		return false;
	}

	frame->GetSize(&width, &height);
	m_width = (int)width;
	m_height = (int)height;

	// Pictures can come in all sorts of pixel formats
	// 8-bit indexed, greyscale, 16-bit per channel, no alpha etc.
	// the format converter normalizes whatever we were handed into one known format.
	hResult = wicFactory->CreateFormatConverter(&converter);
	if (FAILED(hResult))
	{
		frame->Release();
		decoder->Release();
		wicFactory->Release();
		return false;
	}

	hResult = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
	if (FAILED(hResult))
	{
		converter->Release();
		frame->Release();
		decoder->Release();
		wicFactory->Release();
		return false;
	}

	stride = m_width * 4;
	bufferSize = stride * m_height;

	m_imageData = new unsigned char[bufferSize];
	if (!m_imageData)
	{
		converter->Release();
		frame->Release();
		decoder->Release();
		wicFactory->Release();
		return false;
	}

	hResult = converter->CopyPixels(NULL, stride, bufferSize, m_imageData);
	if (FAILED(hResult))
	{
		delete[] m_imageData;
		m_imageData = 0;
		converter->Release();
		frame->Release();
		decoder->Release();
		wicFactory->Release();
		return false;
	}

	converter->Release();
	frame->Release();
	decoder->Release();
	wicFactory->Release();

	return true;
}