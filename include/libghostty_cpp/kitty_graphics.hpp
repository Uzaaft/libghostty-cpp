#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "libghostty_cpp/bytes.hpp"
#include "libghostty_cpp/vt.hpp"

namespace libghostty_cpp {
class Terminal;
}

namespace libghostty_cpp::kitty_graphics {

class Graphics;
class Placement;

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

struct Selection {
  PointCoordinate start;
  PointCoordinate end;
  bool rectangle;
};

/// Metadata for a Kitty graphics image.
///
/// The bytes member is a borrowed view into Ghostty-owned image storage. It is
/// invalidated when the owning terminal mutates, resets, or is destroyed. Copy
/// the bytes before retaining them beyond the immediate render pass.
struct ImageInfo {
  std::uint32_t id;
  std::uint32_t number;
  std::uint32_t width;
  std::uint32_t height;
  ImageFormat format;
  Compression compression;
  ByteView bytes;
};

/// Metadata for a single Kitty graphics placement.
///
/// Placement values are read from the iterator's current position. A Placement
/// becomes invalid as soon as its PlacementIterator advances or is rebound.
struct PlacementInfo {
  std::uint32_t image_id;
  std::uint32_t placement_id;
  bool is_virtual;
  std::uint32_t x_offset;
  std::uint32_t y_offset;
  std::uint32_t source_x;
  std::uint32_t source_y;
  std::uint32_t source_width;
  std::uint32_t source_height;
  std::uint32_t columns;
  std::uint32_t rows;
  std::int32_t z;
};

/// Render-facing values for a Kitty graphics placement.
///
/// The optional viewport_pos and rect values are empty when Ghostty cannot map
/// the placement into that coordinate space, for example virtual placements or
/// placements outside the requested point kind.
struct PlacementRenderInfo {
  PixelSize pixel_size;
  GridSize grid_size;
  std::optional<ViewportPos> viewport_pos;
  SourceRect source_rect;
  std::optional<Selection> rect;
};

class Image {
public:
  [[nodiscard]] std::uint32_t id() const;
  [[nodiscard]] std::uint32_t number() const;
  [[nodiscard]] std::uint32_t width() const;
  [[nodiscard]] std::uint32_t height() const;
  [[nodiscard]] ImageFormat format() const;
  [[nodiscard]] Compression compression() const;
  [[nodiscard]] ByteView bytes() const;
  [[nodiscard]] ImageInfo info() const;

private:
  explicit Image(void* handle) noexcept : handle_(handle) {}

  void ensure_handle() const;

  void* handle_ = nullptr;

  friend class Graphics;
  friend class Placement;
};

class Placement {
public:
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
  [[nodiscard]] std::optional<Selection> rect(
    const Image& image,
    const Terminal& terminal,
    Point::Kind point_kind = Point::Kind::Viewport
  ) const;
  [[nodiscard]] PlacementInfo info() const;
  [[nodiscard]] PlacementRenderInfo render_info(
    const Image& image,
    const Terminal& terminal,
    Point::Kind point_kind = Point::Kind::Viewport
  ) const;

private:
  explicit Placement(const class PlacementIterator* iterator, std::size_t revision) noexcept
      : iterator_(iterator), revision_(revision) {}

  void ensure_current() const;
  [[nodiscard]] void* handle() const;

  const class PlacementIterator* iterator_ = nullptr;
  std::size_t revision_ = 0;

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

  PlacementIterator& bind(const Graphics& graphics, PlacementLayer layer = PlacementLayer::All);
  [[nodiscard]] std::optional<Placement> next();

private:
  void ensure_handle() const;
  void release() noexcept;

  void* handle_ = nullptr;
  std::size_t revision_ = 0;
  bool positioned_ = false;

  friend class Placement;
  friend class Graphics;
};

class Graphics {
public:
  [[nodiscard]] std::optional<Image> find_image(std::uint32_t image_id) const;
  [[nodiscard]] PlacementIterator placements(PlacementLayer layer = PlacementLayer::All) const;

private:
  explicit Graphics(void* handle) noexcept : handle_(handle) {}

  void* handle_ = nullptr;

  friend class ::libghostty_cpp::Terminal;
  friend class PlacementIterator;
};

} // namespace libghostty_cpp::kitty_graphics
