#pragma once

#include <cstdint>
#include <optional>

#include "libghostty_cpp/error.hpp"
#include "libghostty_cpp/style.hpp"

namespace libghostty_cpp {
class Terminal;
}

namespace libghostty_cpp::kitty_graphics {

enum class ImageFormat {
  Rgb,
  Rgba,
  Png,
  GrayAlpha,
  Gray,
};

enum class Compression {
  None,
  ZlibDeflate,
};

enum class PlacementLayer {
  All,
  BelowBg,
  BelowText,
  AboveText,
};

struct PixelSize {
  std::uint32_t width;
  std::uint32_t height;
};

struct GridSize {
  std::uint32_t cols;
  std::uint32_t rows;
};

struct ViewportPos {
  std::int32_t col;
  std::int32_t row;
};

struct SourceRect {
  std::uint32_t x;
  std::uint32_t y;
  std::uint32_t width;
  std::uint32_t height;
};

class Image {
public:
  [[nodiscard]] std::uint32_t id() const;
  [[nodiscard]] std::uint32_t number() const;
  [[nodiscard]] std::uint32_t width() const;
  [[nodiscard]] std::uint32_t height() const;
  [[nodiscard]] ImageFormat format() const;
  [[nodiscard]] Compression compression() const;
  [[nodiscard]] const std::uint8_t* data() const;
  [[nodiscard]] std::size_t data_len() const;

private:
  explicit Image(void* handle) noexcept : handle_(handle) {}

  void* handle_ = nullptr;

  friend class Graphics;
  friend class PlacementIterator;
};

class PlacementIterator {
public:
  PlacementIterator();
  ~PlacementIterator();

  PlacementIterator(PlacementIterator&& other) noexcept;
  PlacementIterator& operator=(PlacementIterator&& other) noexcept;

  PlacementIterator(const PlacementIterator&) = delete;
  PlacementIterator& operator=(const PlacementIterator&) = delete;

  PlacementIterator& set_layer(PlacementLayer layer);
  [[nodiscard]] bool next();

  [[nodiscard]] std::uint32_t image_id() const;
  [[nodiscard]] std::uint32_t placement_id() const;
  [[nodiscard]] bool is_virtual() const;
  [[nodiscard]] std::uint32_t x_offset() const;
  [[nodiscard]] std::uint32_t y_offset() const;
  [[nodiscard]] std::uint32_t source_x() const;
  [[nodiscard]] std::uint32_t source_y() const;
  [[nodiscard]] std::uint32_t source_width() const;
  [[nodiscard]] std::uint32_t source_height() const;
  [[nodiscard]] std::uint32_t columns() const;
  [[nodiscard]] std::uint32_t rows() const;
  [[nodiscard]] std::int32_t z() const;

  [[nodiscard]] PixelSize pixel_size(const Image& image, const Terminal& terminal) const;
  [[nodiscard]] GridSize grid_size(const Image& image, const Terminal& terminal) const;
  [[nodiscard]] std::optional<ViewportPos> viewport_pos(const Image& image, const Terminal& terminal) const;
  [[nodiscard]] SourceRect source_rect(const Image& image) const;

private:
  void ensure_handle() const;
  void release() noexcept;

  void* handle_ = nullptr;

  friend class Graphics;
};

class Graphics {
public:
  [[nodiscard]] Image image(std::uint32_t image_id) const;
  [[nodiscard]] PlacementIterator placements() const;

private:
  explicit Graphics(void* handle) noexcept : handle_(handle) {}

  void* handle_ = nullptr;

  friend class ::libghostty_cpp::Terminal;
};

} // namespace libghostty_cpp::kitty_graphics
