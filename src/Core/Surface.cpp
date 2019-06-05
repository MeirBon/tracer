#include "Surface.h"
#include "Utils/Messages.h"

namespace core
{

char Surface::s_Font[51][5][6];
bool Surface::fontInitialized = false;

Surface::Surface(int width, int height, Pixel *buffer, int pitch)
	: m_Buffer(buffer), m_Width(width), m_Height(height), m_Pitch(pitch)
{
	m_TexBuffer = {glm::vec4(1.0f)};
	m_Flags = 0;
}

Surface::Surface(int width, int height) : m_Width(width), m_Height(height), m_Pitch(width)
{
	m_TexBuffer = {glm::vec4(1.0f)};
	m_Buffer = new Pixel[width * height];
	m_Flags = OWNER;
}

Surface::Surface(const char *file) : m_Buffer(nullptr), m_Width(0), m_Height(0)
{
	m_TexBuffer = {glm::vec4(1.0f)};
	FILE *f = fopen(file, "rb");
	if (!f)
	{
		std::string t = std::string("File not found: ") + file;
		const char *msg = t.c_str();
		utils::WarningMessage(__FILE__, __LINE__, msg, "Surface");
		return;
	}
	else
		fclose(f);
	LoadImage(file);
}

void Surface::LoadImage(const char *file)
{
	using namespace glm;

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(file, 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(file);

	FIBITMAP *tmp = FreeImage_Load(fif, file);
	FIBITMAP *dib = FreeImage_ConvertTo32Bits(tmp);
	FreeImage_Unload(tmp);

	m_Width = m_Pitch = FreeImage_GetWidth(dib);
	m_Height = FreeImage_GetHeight(dib);
	m_Buffer = new Pixel[m_Width * m_Height];
	m_TexBuffer.reserve(m_Width * m_Height);
	m_Flags = OWNER;

	for (auto y = 0; y < m_Height; y++)
	{
		for (auto x = 0; x < m_Width; x++)
		{
			RGBQUAD quad;
			FreeImage_GetPixelColor(dib, x, y, &quad);
			const auto red = (uint)(quad.rgbRed);
			const auto green = (uint)(quad.rgbGreen);
			const auto blue = (uint)(quad.rgbBlue);
			m_Buffer[x + y * m_Width] = (red << 0) | (green << 8) | (blue << 16);
			m_TexBuffer.emplace_back(float(red) / 255.99f, float(green) / 255.99f, float(blue) / 255.99f, 1.0f);
		}
	}

	FreeImage_Unload(dib);
}

Surface::~Surface()
{
	if (m_Flags & OWNER)
		delete[] m_Buffer;
}

void Surface::SetBuffer(Pixel *buffer)
{
	if (m_Flags == OWNER)
	{
		delete[] m_Buffer;
		m_Flags = 0;
	}

	m_Buffer = buffer;
}

void Surface::Clear(Pixel color)
{
	int s = m_Width * m_Height;
	for (int i = 0; i < s; i++)
		m_Buffer[i] = color;
}

void Surface::Centre(const char *str, int y1, Pixel color)
{
	int x = (m_Width - (int)strlen(str) * 6) / 2;
	Print(str, x, y1, color);
}

void Surface::Print(const char *str, int x1, int y1, Pixel color)
{
	if (!fontInitialized)
	{
		InitCharset();
		fontInitialized = true;
	}
	Pixel *t = m_Buffer + x1 + y1 * m_Pitch;
	for (int i = 0; i < (int)(strlen(str)); i++, t += 6)
	{
		long pos = 0;
		if ((str[i] >= 'A') && (str[i] <= 'Z'))
			pos = s_Transl[(unsigned short)(str[i] - ('A' - 'a'))];
		else
			pos = s_Transl[(unsigned short)str[i]];
		Pixel *a = t;
		const char *c = (const char *)s_Font[pos];
		for (int v = 0; v < 5; v++, c++, a += m_Pitch)
			for (int h = 0; h < 5; h++)
				if (*c++ == 'o')
					*(a + h) = color, *(a + h + m_Pitch) = 0;
	}
}

void Surface::Resize(Surface *original)
{
	Pixel *src = original->GetBuffer(), *dst = m_Buffer;
	int u, v, owidth = original->getWidth(), oheight = original->getHeight();
	int dx = (owidth << 10) / m_Width, dy = (oheight << 10) / m_Height;
	for (v = 0; v < m_Height; v++)
	{
		for (u = 0; u < m_Width; u++)
		{
			int su = u * dx, sv = v * dy;
			Pixel *s = src + (su >> 10) + (sv >> 10) * owidth;
			int ufrac = su & 1023, vfrac = sv & 1023;
			int w4 = (ufrac * vfrac) >> 12;
			int w3 = ((1023 - ufrac) * vfrac) >> 12;
			int w2 = (ufrac * (1023 - vfrac)) >> 12;
			int w1 = ((1023 - ufrac) * (1023 - vfrac)) >> 12;
			int x2 = ((su + dx) > ((owidth - 1) << 10)) ? 0 : 1;
			int y2 = ((sv + dy) > ((oheight - 1) << 10)) ? 0 : 1;
			Pixel p1 = *s, p2 = *(s + x2), p3 = *(s + owidth * y2), p4 = *(s + owidth * y2 + x2);
			unsigned int r =
				(((p1 & REDMASK) * w1 + (p2 & REDMASK) * w2 + (p3 & REDMASK) * w3 + (p4 & REDMASK) * w4) >> 8) &
				REDMASK;
			unsigned int g =
				(((p1 & GREENMASK) * w1 + (p2 & GREENMASK) * w2 + (p3 & GREENMASK) * w3 + (p4 & GREENMASK) * w4) >> 8) &
				GREENMASK;
			unsigned int b =
				(((p1 & BLUEMASK) * w1 + (p2 & BLUEMASK) * w2 + (p3 & BLUEMASK) * w3 + (p4 & BLUEMASK) * w4) >> 8) &
				BLUEMASK;
			*(dst + u + v * m_Pitch) = (Pixel)(r + g + b);
		}
	}
}

const glm::vec3 Surface::getColorAt(glm::vec2 texCoords) const
{
	const int yValue = (int)glm::max(0, (int)(glm::min(texCoords.y, 1.f) * m_Height) - 1);
	const int xValue = (int)glm::max(0, (int)(glm::min(texCoords.x, 1.f) * m_Width) - 1);

	return m_TexBuffer[xValue + yValue * m_Pitch];
}

const glm::vec3 Surface::getColorAt(const float &x, const float &y) const
{
	const int yValue = (int)glm::max(0, (int)(glm::min(y, 1.f) * m_Height) - 1);
	const int xValue = (int)glm::max(0, (int)(glm::min(x, 1.f) * m_Width) - 1);

	return m_TexBuffer[xValue + yValue * m_Pitch];
}

#define OUTCODE(x, y) (((x) < xmin) ? 1 : (((x) > xmax) ? 2 : 0)) + (((y) < ymin) ? 4 : (((y) > ymax) ? 8 : 0))

void Surface::Line(float x1, float y1, float x2, float y2, Pixel c)
{
	// clip (Cohen-Sutherland,
	// https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm)
	const float xmin = 0, ymin = 0, xmax = (float)m_Width - 1, ymax = (float)m_Height - 1;
	int c0 = OUTCODE(x1, y1), c1 = OUTCODE(x2, y2);
	bool accept = false;
	while (1)
	{
		if (!(c0 | c1))
		{
			accept = true;
			break;
		}
		else if (c0 & c1)
			break;
		else
		{
			float x, y;
			const int co = c0 ? c0 : c1;
			if (co & 8)
				x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1), y = ymax;
			else if (co & 4)
				x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1), y = ymin;
			else if (co & 2)
				y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1), x = xmax;
			else if (co & 1)
				y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1), x = xmin;
			if (co == c0)
				x1 = x, y1 = y, c0 = OUTCODE(x1, y1);
			else
				x2 = x, y2 = y, c1 = OUTCODE(x2, y2);
		}
	}
	if (!accept)
		return;
	float b = x2 - x1;
	float h = y2 - y1;
	float l = fabsf(b);
	if (fabsf(h) > l)
		l = fabsf(h);
	int il = (int)l;
	float dx = b / (float)l;
	float dy = h / (float)l;
	for (int i = 0; i <= il; i++)
	{
		*(m_Buffer + (int)x1 + (int)y1 * m_Pitch) = c;
		x1 += dx, y1 += dy;
	}
}

void Surface::Plot(int x, int y, Pixel c) { m_Buffer[x + y * m_Pitch] = c; }

void Surface::Plot(int x, int y, const glm::vec3 &color)
{
	glm::vec3 col = glm::sqrt(glm::clamp(color, 0.0f, 1.0f)) * 255.99f;
	const unsigned int red = (unsigned int)(col.r);
	const unsigned int green = (unsigned int)(col.g) << 8;
	const unsigned int blue = (unsigned int)(col.b) << 16;
	Plot(x, y, red + green + blue);
}

void Surface::Box(int x1, int y1, int x2, int y2, Pixel c)
{
	Line((float)x1, (float)y1, (float)x2, (float)y1, c);
	Line((float)x2, (float)y1, (float)x2, (float)y2, c);
	Line((float)x1, (float)y2, (float)x2, (float)y2, c);
	Line((float)x1, (float)y1, (float)x1, (float)y2, c);
}

void Surface::Bar(int x1, int y1, int x2, int y2, Pixel c)
{
	Pixel *a = x1 + y1 * m_Pitch + m_Buffer;
	for (int y = y1; y <= y2; y++)
	{
		for (int x = 0; x <= (x2 - x1); x++)
			a[x] = c;
		a += m_Pitch;
	}
}

void Surface::CopyTo(Surface *destination, int a_X, int a_Y)
{
	Pixel *dst = destination->GetBuffer();
	Pixel *src = m_Buffer;
	if ((src) && (dst))
	{
		int srcwidth = m_Width;
		int srcheight = m_Height;
		int srcpitch = m_Pitch;
		int dstwidth = destination->getWidth();
		int dstheight = destination->getHeight();
		int dstpitch = destination->getPitch();
		if ((srcwidth + a_X) > dstwidth)
			srcwidth = dstwidth - a_X;
		if ((srcheight + a_Y) > dstheight)
			srcheight = dstheight - a_Y;
		if (a_X < 0)
			src -= a_X, srcwidth += a_X, a_X = 0;
		if (a_Y < 0)
			src -= a_Y * srcpitch, srcheight += a_Y, a_Y = 0;
		if ((srcwidth > 0) && (srcheight > 0))
		{
			dst += a_X + dstpitch * a_Y;
			for (int y = 0; y < srcheight; y++)
			{
				memcpy(dst, src, srcwidth * 4);
				dst += dstpitch;
				src += srcpitch;
			}
		}
	}
}

void Surface::BlendCopyTo(Surface *destination, int X, int Y)
{
	Pixel *dst = destination->GetBuffer();
	Pixel *src = m_Buffer;
	if ((src) && (dst))
	{
		int srcwidth = m_Width;
		int srcheight = m_Height;
		int srcpitch = m_Pitch;
		int dstwidth = destination->getWidth();
		int dstheight = destination->getHeight();
		int dstpitch = destination->getPitch();
		if ((srcwidth + X) > dstwidth)
			srcwidth = dstwidth - X;
		if ((srcheight + Y) > dstheight)
			srcheight = dstheight - Y;
		if (X < 0)
			src -= X, srcwidth += X, X = 0;
		if (Y < 0)
			src -= Y * srcpitch, srcheight += Y, Y = 0;
		if ((srcwidth > 0) && (srcheight > 0))
		{
			dst += X + dstpitch * Y;
			for (int y = 0; y < srcheight; y++)
			{
				for (int x = 0; x < srcwidth; x++)
					dst[x] = AddBlend(dst[x], src[x]);
				dst += dstpitch;
				src += srcpitch;
			}
		}
	}
}

void Surface::SetChar(int c, const char *c1, const char *c2, const char *c3, const char *c4, const char *c5)
{
	strcpy(s_Font[c][0], c1);
	strcpy(s_Font[c][1], c2);
	strcpy(s_Font[c][2], c3);
	strcpy(s_Font[c][3], c4);
	strcpy(s_Font[c][4], c5);
}

void Surface::InitCharset()
{
	SetChar(0, ":ooo:", "o:::o", "ooooo", "o:::o", "o:::o");
	SetChar(1, "oooo:", "o:::o", "oooo:", "o:::o", "oooo:");
	SetChar(2, ":oooo", "o::::", "o::::", "o::::", ":oooo");
	SetChar(3, "oooo:", "o:::o", "o:::o", "o:::o", "oooo:");
	SetChar(4, "ooooo", "o::::", "oooo:", "o::::", "ooooo");
	SetChar(5, "ooooo", "o::::", "ooo::", "o::::", "o::::");
	SetChar(6, ":oooo", "o::::", "o:ooo", "o:::o", ":ooo:");
	SetChar(7, "o:::o", "o:::o", "ooooo", "o:::o", "o:::o");
	SetChar(8, "::o::", "::o::", "::o::", "::o::", "::o::");
	SetChar(9, ":::o:", ":::o:", ":::o:", ":::o:", "ooo::");
	SetChar(10, "o::o:", "o:o::", "oo:::", "o:o::", "o::o:");
	SetChar(11, "o::::", "o::::", "o::::", "o::::", "ooooo");
	SetChar(12, "oo:o:", "o:o:o", "o:o:o", "o:::o", "o:::o");
	SetChar(13, "o:::o", "oo::o", "o:o:o", "o::oo", "o:::o");
	SetChar(14, ":ooo:", "o:::o", "o:::o", "o:::o", ":ooo:");
	SetChar(15, "oooo:", "o:::o", "oooo:", "o::::", "o::::");
	SetChar(16, ":ooo:", "o:::o", "o:::o", "o::oo", ":oooo");
	SetChar(17, "oooo:", "o:::o", "oooo:", "o:o::", "o::o:");
	SetChar(18, ":oooo", "o::::", ":ooo:", "::::o", "oooo:");
	SetChar(19, "ooooo", "::o::", "::o::", "::o::", "::o::");
	SetChar(20, "o:::o", "o:::o", "o:::o", "o:::o", ":oooo");
	SetChar(21, "o:::o", "o:::o", ":o:o:", ":o:o:", "::o::");
	SetChar(22, "o:::o", "o:::o", "o:o:o", "o:o:o", ":o:o:");
	SetChar(23, "o:::o", ":o:o:", "::o::", ":o:o:", "o:::o");
	SetChar(24, "o:::o", "o:::o", ":oooo", "::::o", ":ooo:");
	SetChar(25, "ooooo", ":::o:", "::o::", ":o:::", "ooooo");
	SetChar(26, ":ooo:", "o::oo", "o:o:o", "oo::o", ":ooo:");
	SetChar(27, "::o::", ":oo::", "::o::", "::o::", ":ooo:");
	SetChar(28, ":ooo:", "o:::o", "::oo:", ":o:::", "ooooo");
	SetChar(29, "oooo:", "::::o", "::oo:", "::::o", "oooo:");
	SetChar(30, "o::::", "o::o:", "ooooo", ":::o:", ":::o:");
	SetChar(31, "ooooo", "o::::", "oooo:", "::::o", "oooo:");
	SetChar(32, ":oooo", "o::::", "oooo:", "o:::o", ":ooo:");
	SetChar(33, "ooooo", "::::o", ":::o:", "::o::", "::o::");
	SetChar(34, ":ooo:", "o:::o", ":ooo:", "o:::o", ":ooo:");
	SetChar(35, ":ooo:", "o:::o", ":oooo", "::::o", ":ooo:");
	SetChar(36, "::o::", "::o::", "::o::", ":::::", "::o::");
	SetChar(37, ":ooo:", "::::o", ":::o:", ":::::", "::o::");
	SetChar(38, ":::::", ":::::", "::o::", ":::::", "::o::");
	SetChar(39, ":::::", ":::::", ":ooo:", ":::::", ":ooo:");
	SetChar(40, ":::::", ":::::", ":::::", ":::o:", "::o::");
	SetChar(41, ":::::", ":::::", ":::::", ":::::", "::o::");
	SetChar(42, ":::::", ":::::", ":ooo:", ":::::", ":::::");
	SetChar(43, ":::o:", "::o::", "::o::", "::o::", ":::o:");
	SetChar(44, "::o::", ":::o:", ":::o:", ":::o:", "::o::");
	SetChar(45, ":::::", ":::::", ":::::", ":::::", ":::::");
	SetChar(46, "ooooo", "ooooo", "ooooo", "ooooo", "ooooo");
	SetChar(47, "::o::", "::o::", ":::::", ":::::", ":::::"); // Tnx Ferry
	SetChar(48, "o:o:o", ":ooo:", "ooooo", ":ooo:", "o:o:o");
	SetChar(49, "::::o", ":::o:", "::o::", ":o:::", "o::::");
	char c[] = "abcdefghijklmnopqrstuvwxyz0123456789!?:=,.-() #'*/";
	int i;
	for (i = 0; i < 256; i++)
		s_Transl[i] = 45;
	for (i = 0; i < 50; i++)
		s_Transl[(unsigned char)c[i]] = i;
}

void Surface::ScaleColor(unsigned int scale)
{
	int s = m_Pitch * m_Height;
	for (int i = 0; i < s; i++)
	{
		Pixel c = m_Buffer[i];
		unsigned int rb = (((c & (REDMASK | BLUEMASK)) * scale) >> 5) & (REDMASK | BLUEMASK);
		unsigned int g = (((c & GREENMASK) * scale) >> 5) & GREENMASK;
		m_Buffer[i] = rb + g;
	}
}

glm::vec4 *Surface::GetTextureBuffer() { return m_TexBuffer.data(); }

Sprite::Sprite(Surface *output, unsigned int numFrames)
	: m_Width(output->getWidth() / numFrames), m_Height(output->getHeight()), m_Pitch(output->getWidth()),
	  m_NumFrames(numFrames), m_CurrentFrame(0), m_Flags(0), m_Start(new unsigned int *[numFrames]), m_Surface(output)
{
	InitializeStartData();
}

Sprite::~Sprite()
{
	delete m_Surface;
	for (unsigned int i = 0; i < m_NumFrames; i++)
		delete m_Start[i];
	delete[] m_Start;
}

void Sprite::Draw(Surface *output, int X, int Y)
{
	if ((X < -m_Width) || (X > (output->getWidth() + m_Width)))
		return;
	if ((Y < -m_Height) || (Y > (output->getHeight() + m_Height)))
		return;
	int x1 = X, x2 = X + m_Width;
	int y1 = Y, y2 = Y + m_Height;
	Pixel *src = GetBuffer() + m_CurrentFrame * m_Width;
	if (x1 < 0)
	{
		src += -x1;
		x1 = 0;
	}
	if (x2 > output->getWidth())
		x2 = output->getWidth();
	if (y1 < 0)
	{
		src += -y1 * m_Pitch;
		y1 = 0;
	}
	if (y2 > output->getHeight())
		y2 = output->getHeight();
	Pixel *dest = output->GetBuffer();
	int xs;
	const int dpitch = output->getPitch();
	if ((x2 > x1) && (y2 > y1))
	{
		unsigned int addr = y1 * dpitch + x1;
		const int width = x2 - x1;
		const int height = y2 - y1;
		for (int y = 0; y < height; y++)
		{
			const int line = y + (y1 - Y);
			const int lsx = m_Start[m_CurrentFrame][line] + X;
			if (m_Flags & FLARE)
			{
				xs = (lsx > x1) ? lsx - x1 : 0;
				for (int x = xs; x < width; x++)
				{
					const Pixel c1 = *(src + x);
					if (c1 & 0xffffff)
					{
						const Pixel c2 = *(dest + addr + x);
						*(dest + addr + x) = AddBlend(c1, c2);
					}
				}
			}
			else
			{
				xs = (lsx > x1) ? lsx - x1 : 0;
				for (int x = xs; x < width; x++)
				{
					const Pixel c1 = *(src + x);
					if (c1 & 0xffffff)
						*(dest + addr + x) = c1;
				}
			}
			addr += dpitch;
			src += m_Pitch;
		}
	}
}

void Sprite::DrawScaled(int X, int Y, int width, int height, Surface *output)
{
	if ((width == 0) || (height == 0))
		return;
	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
		{
			int u = (int)((float)x * ((float)m_Width / (float)width));
			int v = (int)((float)y * ((float)m_Height / (float)height));
			Pixel color = GetBuffer()[u + v * m_Pitch];
			if (color & 0xffffff)
				output->GetBuffer()[X + x + ((Y + y) * output->getPitch())] = color;
		}
}

void Sprite::InitializeStartData()
{
	for (unsigned int f = 0; f < m_NumFrames; ++f)
	{
		m_Start[f] = new unsigned int[m_Height];
		for (int y = 0; y < m_Height; ++y)
		{
			m_Start[f][y] = m_Width;
			Pixel *addr = GetBuffer() + f * m_Width + y * m_Pitch;
			for (int x = 0; x < m_Width; ++x)
			{
				if (addr[x])
				{
					m_Start[f][y] = x;
					break;
				}
			}
		}
	}
}

SurfaceFont::SurfaceFont(const char *file, const char *chars)
{
	m_Surface = new Surface(file);
	Pixel *b = m_Surface->GetBuffer();
	int w = m_Surface->getWidth();
	int h = m_Surface->getHeight();
	unsigned int charnr = 0, start = 0;
	m_Trans = new int[256];
	memset(m_Trans, 0, 1024);
	unsigned int i;
	for (i = 0; i < strlen(chars); i++)
		m_Trans[(unsigned char)chars[i]] = i;
	m_Offset = new int[strlen(chars)];
	m_Width = new int[strlen(chars)];
	m_Height = h;
	m_CY1 = 0, m_CY2 = 1024;
	int x, y;
	bool lastempty = true;
	for (x = 0; x < w; x++)
	{
		bool empty = true;
		for (y = 0; y < h; y++)
			if (*(b + x + y * w) & 0xffffff)
			{
				if (lastempty)
					start = x;
				empty = false;
			}
		if ((empty) && (!lastempty))
		{
			m_Width[charnr] = x - start;
			m_Offset[charnr] = start;
			if (++charnr == strlen(chars))
				break;
		}
		lastempty = empty;
	}
}

SurfaceFont::~SurfaceFont()
{
	delete m_Surface;
	delete m_Trans;
	delete m_Width;
	delete m_Offset;
}

int SurfaceFont::Width(const char *text)
{
	int w = 0;
	unsigned int i;
	for (i = 0; i < strlen(text); i++)
	{
		unsigned char c = (unsigned char)text[i];
		if (c == 32)
			w += 4;
		else
			w += m_Width[m_Trans[c]] + 2;
	}
	return w;
}

void SurfaceFont::Centre(Surface *output, const char *text, int y)
{
	int x = (output->getPitch() - Width(text)) / 2;
	Print(output, text, x, y);
}

void SurfaceFont::Print(Surface *output, const char *text, int X, int Y, bool clip)
{
	Pixel *b = output->GetBuffer() + X + Y * output->getPitch();
	Pixel *s = m_Surface->GetBuffer();
	unsigned int i, cx;
	int x, y;
	if (((Y + m_Height) < m_CY1) || (Y > m_CY2))
		return;
	for (cx = 0, i = 0; i < strlen(text); i++)
	{
		if (text[i] == ' ')
			cx += 4;
		else
		{
			int c = m_Trans[(unsigned char)text[i]];
			Pixel *t = s + m_Offset[c], *d = b + cx;
			if (clip)
			{
				for (y = 0; y < m_Height; y++)
				{
					if (((Y + y) >= m_CY1) && ((Y + y) <= m_CY2))
					{
						for (x = 0; x < m_Width[c]; x++)
							if ((t[x]) && ((x + (int)cx + X) < output->getPitch()))
								d[x] = AddBlend(t[x], d[x]);
					}
					t += m_Surface->getPitch(), d += output->getPitch();
				}
			}
			else
			{
				for (y = 0; y < m_Height; y++)
				{
					if (((Y + y) >= m_CY1) && ((Y + y) <= m_CY2))
						for (x = 0; x < m_Width[c]; x++)
							if (t[x])
								d[x] = AddBlend(t[x], d[x]);
					t += m_Surface->getPitch(), d += output->getPitch();
				}
			}
			cx += m_Width[c] + 2;
			if ((int)(cx + X) >= output->getPitch())
				break;
		}
	}
}
} // namespace core
