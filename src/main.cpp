#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// A small 24-bit, uncompressed BMP reader/writer (BGR order and row padding).
struct Image
{
  int width{}, height{};
  std::vector<uint8_t> rgb;
};
uint16_t u16(const std::vector<uint8_t> &b, size_t i) { return uint16_t(b.at(i)) | uint16_t(b.at(i + 1)) << 8; }
uint32_t u32(const std::vector<uint8_t> &b, size_t i) { return uint32_t(b.at(i)) | uint32_t(b.at(i + 1)) << 8 | uint32_t(b.at(i + 2)) << 16 | uint32_t(b.at(i + 3)) << 24; }
void put16(std::vector<uint8_t> &b, size_t i, uint16_t v)
{
  b[i] = v & 255;
  b[i + 1] = (v >> 8) & 255;
}
void put32(std::vector<uint8_t> &b, size_t i, uint32_t v)
{
  for (int j = 0; j < 4; j++)
    b[i + j] = (v >> (8 * j)) & 255;
}
Image loadBMP(const std::string &name)
{
  std::ifstream in(name, std::ios::binary);
  if (!in)
    throw std::runtime_error("Cannot open: " + name);
  std::vector<uint8_t> b((std::istreambuf_iterator<char>(in)), {});
  if (b.size() < 54 || b[0] != 'B' || b[1] != 'M' || u32(b, 14) < 40 || u16(b, 28) != 24 || u32(b, 30) != 0)
    throw std::runtime_error("Requires uncompressed 24-bit BMP");
  int32_t w = static_cast<int32_t>(u32(b, 18)), h = static_cast<int32_t>(u32(b, 22));
  if (w <= 0 || h == 0 || h == INT32_MIN || w > 10000 || h > 10000 || h < -10000)
    throw std::runtime_error("Invalid BMP size");
  int height = h < 0 ? -h : h;
  size_t row = (static_cast<size_t>(w) * 3 + 3) & ~size_t(3), offset = u32(b, 10);
  if (offset > b.size() || row * height > b.size() - offset)
    throw std::runtime_error("Truncated BMP");
  Image img{w, height, std::vector<uint8_t>(size_t(w) * height * 3)};
  for (int y = 0; y < height; y++)
  {
    int srcY = h > 0 ? height - 1 - y : y;
    for (int x = 0; x < w; x++)
      for (int c = 0; c < 3; c++)
        img.rgb[(size_t(y) * w + x) * 3 + c] = b[offset + size_t(srcY) * row + x * 3 + c];
  }
  return img;
}
void saveBMP(const Image &a, const std::string &name)
{
  size_t row = (size_t(a.width) * 3 + 3) & ~size_t(3), bytes = 54 + row * a.height;
  std::vector<uint8_t> b(bytes, 0);
  b[0] = 'B';
  b[1] = 'M';
  put32(b, 2, bytes);
  put32(b, 10, 54);
  put32(b, 14, 40);
  put32(b, 18, a.width);
  put32(b, 22, a.height);
  put16(b, 26, 1);
  put16(b, 28, 24);
  put32(b, 34, row * a.height);
  for (int y = 0; y < a.height; y++)
    for (int x = 0; x < a.width; x++)
      for (int c = 0; c < 3; c++)
        b[54 + size_t(a.height - 1 - y) * row + x * 3 + c] = a.rgb[(size_t(y) * a.width + x) * 3 + c];
  std::ofstream out(name, std::ios::binary);
  if (!out)
    throw std::runtime_error("Cannot write: " + name);
  out.write(reinterpret_cast<const char *>(b.data()), b.size());
}
Image gray(const Image &src)
{
  Image out = src;
  for (size_t i = 0; i < out.rgb.size(); i += 3)
  {
    int v = std::clamp(int(0.114 * src.rgb[i] + 0.587 * src.rgb[i + 1] + 0.299 * src.rgb[i + 2] + 0.5), 0, 255);
    out.rgb[i] = out.rgb[i + 1] = out.rgb[i + 2] = v;
  }
  return out;
}
Image blurInverted(const Image &g)
{
  Image out = g;
  const int w = g.width, h = g.height;
  for (int y = 0; y < h; y++)
    for (int x = 0; x < w; x++)
    {
      int sum = 0, count = 0;
      for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
        {
          int xx = x + dx, yy = y + dy;
          if (xx >= 0 && xx < w && yy >= 0 && yy < h)
          {
            sum += 255 - g.rgb[(size_t(yy) * w + xx) * 3];
            count++;
          }
        }
      uint8_t v = static_cast<uint8_t>(sum / count);
      size_t i = (size_t(y) * w + x) * 3;
      out.rgb[i] = out.rgb[i + 1] = out.rgb[i + 2] = v;
    }
  return out;
}
Image pencil(const Image &g, const Image &blurredInverse)
{
  Image out = g;
  for (size_t i = 0; i < out.rgb.size(); i += 3)
  {
    int a = g.rgb[i], b = blurredInverse.rgb[i];
    int v = b == 255 ? 255 : std::min(255, a * 255 / (255 - b));
    out.rgb[i] = out.rgb[i + 1] = out.rgb[i + 2] = static_cast<uint8_t>(v);
  }
  return out;
}
int main(int argc, char **argv)
{
  if (argc != 3)
  {
    std::cerr << "Usage: bmp-sketch input.bmp output-prefix\\n";
    return 1;
  }
  try
  {
    Image input = loadBMP(argv[1]);
    Image g = gray(input);
    Image b = blurInverted(g);
    Image s = pencil(g, b);
    saveBMP(g, std::string(argv[2]) + "-gray.bmp");
    saveBMP(s, std::string(argv[2]) + "-sketch.bmp");
    std::cout << "Saved grayscale and pencil-sketch BMP images (" << input.width << "x" << input.height << ")\n";
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
