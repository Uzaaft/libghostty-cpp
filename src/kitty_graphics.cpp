#include "libghostty_cpp/kitty_graphics.hpp"

#include "c_compat/internal.h"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"
#include "libghostty_cpp/vt.hpp"

#include <utility>

namespace libghostty_cpp::kitty_graphics {

namespace {

GhosttyKittyGraphicsPlacementIterator as_iterator(void* handle) {
  return static_cast<GhosttyKittyGraphicsPlacementIterator>(handle);
}

GhosttyKittyGraphicsImage as_image(void* handle) {
  return static_cast<GhosttyKittyGraphicsImage>(handle);
}

GhosttyKittyGraphics as_graphics(void* handle) {
  return static_cast<GhosttyKittyGraphics>(handle);
}

std::uint32_t get_image_u32(
  GhosttyKittyGraphicsImage image,
  GhosttyKittyGraphicsImageData data_kind
) {
  std::uint32_t value = 0;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_image_get(image, data_kind, &value)
  );
  return value;
}

std::uint32_t get_placement_u32(
  GhosttyKittyGraphicsPlacementIterator iterator,
  GhosttyKittyGraphicsPlacementData data_kind
) {
  std::uint32_t value = 0;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_placement_get(iterator, data_kind, &value)
  );
  return value;
}

GhosttyKittyPlacementLayer translate_layer(PlacementLayer layer) {
  switch (layer) {
    case PlacementLayer::All:
      return GHOSTTY_KITTY_PLACEMENT_LAYER_ALL;
    case PlacementLayer::BelowBg:
      return GHOSTTY_KITTY_PLACEMENT_LAYER_BELOW_BG;
    case PlacementLayer::BelowText:
      return GHOSTTY_KITTY_PLACEMENT_LAYER_BELOW_TEXT;
    case PlacementLayer::AboveText:
      return GHOSTTY_KITTY_PLACEMENT_LAYER_ABOVE_TEXT;
  }

  throw Error(ErrorCode::InvalidValue);
}

ImageFormat translate_image_format(GhosttyKittyImageFormat format) {
  switch (format) {
    case GHOSTTY_KITTY_IMAGE_FORMAT_RGB:
      return ImageFormat::Rgb;
    case GHOSTTY_KITTY_IMAGE_FORMAT_RGBA:
      return ImageFormat::Rgba;
    case GHOSTTY_KITTY_IMAGE_FORMAT_PNG:
      return ImageFormat::Png;
    case GHOSTTY_KITTY_IMAGE_FORMAT_GRAY_ALPHA:
      return ImageFormat::GrayAlpha;
    case GHOSTTY_KITTY_IMAGE_FORMAT_GRAY:
      return ImageFormat::Gray;
  }

  throw Error(ErrorCode::InvalidValue);
}

Compression translate_compression(GhosttyKittyImageCompression compression) {
  switch (compression) {
    case GHOSTTY_KITTY_IMAGE_COMPRESSION_NONE:
      return Compression::None;
    case GHOSTTY_KITTY_IMAGE_COMPRESSION_ZLIB_DEFLATE:
      return Compression::ZlibDeflate;
  }

  throw Error(ErrorCode::InvalidValue);
}

} // namespace

// --- Image ---

std::uint32_t Image::id() const {
  return get_image_u32(as_image(handle_), GHOSTTY_KITTY_IMAGE_DATA_ID);
}

std::uint32_t Image::number() const {
  return get_image_u32(as_image(handle_), GHOSTTY_KITTY_IMAGE_DATA_NUMBER);
}

std::uint32_t Image::width() const {
  return get_image_u32(as_image(handle_), GHOSTTY_KITTY_IMAGE_DATA_WIDTH);
}

std::uint32_t Image::height() const {
  return get_image_u32(as_image(handle_), GHOSTTY_KITTY_IMAGE_DATA_HEIGHT);
}

ImageFormat Image::format() const {
  GhosttyKittyImageFormat raw_format = GHOSTTY_KITTY_IMAGE_FORMAT_RGB;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_image_get(
      as_image(handle_), GHOSTTY_KITTY_IMAGE_DATA_FORMAT, &raw_format
    )
  );
  return translate_image_format(raw_format);
}

Compression Image::compression() const {
  GhosttyKittyImageCompression raw_compression = GHOSTTY_KITTY_IMAGE_COMPRESSION_NONE;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_image_get(
      as_image(handle_), GHOSTTY_KITTY_IMAGE_DATA_COMPRESSION, &raw_compression
    )
  );
  return translate_compression(raw_compression);
}

const std::uint8_t* Image::data() const {
  const std::uint8_t* ptr = nullptr;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_image_get(
      as_image(handle_), GHOSTTY_KITTY_IMAGE_DATA_DATA_PTR, &ptr
    )
  );
  return ptr;
}

std::size_t Image::data_len() const {
  std::size_t len = 0;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_image_get(
      as_image(handle_), GHOSTTY_KITTY_IMAGE_DATA_DATA_LEN, &len
    )
  );
  return len;
}

// --- PlacementIterator ---

PlacementIterator::PlacementIterator() {
  GhosttyKittyGraphicsPlacementIterator handle = nullptr;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_placement_iterator_new(nullptr, &handle)
  );
  handle_ = handle;
}

PlacementIterator::~PlacementIterator() {
  release();
}

PlacementIterator::PlacementIterator(PlacementIterator&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}

PlacementIterator& PlacementIterator::operator=(PlacementIterator&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  return *this;
}

PlacementIterator& PlacementIterator::set_layer(PlacementLayer layer) {
  ensure_handle();
  GhosttyKittyPlacementLayer raw_layer = translate_layer(layer);
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_placement_iterator_set(
      as_iterator(handle_),
      GHOSTTY_KITTY_GRAPHICS_PLACEMENT_ITERATOR_OPTION_LAYER,
      &raw_layer
    )
  );
  return *this;
}

bool PlacementIterator::next() {
  ensure_handle();
  return ghostty_kitty_graphics_placement_next(as_iterator(handle_));
}

std::uint32_t PlacementIterator::image_id() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_IMAGE_ID
  );
}

std::uint32_t PlacementIterator::placement_id() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_PLACEMENT_ID
  );
}

bool PlacementIterator::is_virtual() const {
  ensure_handle();
  bool value = false;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_placement_get(
      as_iterator(handle_),
      GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_IS_VIRTUAL,
      &value
    )
  );
  return value;
}

std::uint32_t PlacementIterator::x_offset() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_X_OFFSET
  );
}

std::uint32_t PlacementIterator::y_offset() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_Y_OFFSET
  );
}

std::uint32_t PlacementIterator::source_x() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_SOURCE_X
  );
}

std::uint32_t PlacementIterator::source_y() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_SOURCE_Y
  );
}

std::uint32_t PlacementIterator::source_width() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_SOURCE_WIDTH
  );
}

std::uint32_t PlacementIterator::source_height() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_SOURCE_HEIGHT
  );
}

std::uint32_t PlacementIterator::columns() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_COLUMNS
  );
}

std::uint32_t PlacementIterator::rows() const {
  ensure_handle();
  return get_placement_u32(
    as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_ROWS
  );
}

std::int32_t PlacementIterator::z() const {
  ensure_handle();
  std::int32_t value = 0;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_placement_get(
      as_iterator(handle_), GHOSTTY_KITTY_GRAPHICS_PLACEMENT_DATA_Z, &value
    )
  );
  return value;
}

PixelSize PlacementIterator::pixel_size(
  const Image& image,
  const Terminal& terminal
) const {
  ensure_handle();
  std::uint32_t w = 0;
  std::uint32_t h = 0;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_placement_pixel_size(
      as_iterator(handle_),
      as_image(image.handle_),
      terminal.handle_->inner,
      &w, &h
    )
  );
  return PixelSize{w, h};
}

GridSize PlacementIterator::grid_size(
  const Image& image,
  const Terminal& terminal
) const {
  ensure_handle();
  std::uint32_t cols = 0;
  std::uint32_t rows = 0;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_placement_grid_size(
      as_iterator(handle_),
      as_image(image.handle_),
      terminal.handle_->inner,
      &cols, &rows
    )
  );
  return GridSize{cols, rows};
}

std::optional<ViewportPos> PlacementIterator::viewport_pos(
  const Image& image,
  const Terminal& terminal
) const {
  ensure_handle();
  std::int32_t col = 0;
  std::int32_t row = 0;
  const GhosttyResult result = ghostty_kitty_graphics_placement_viewport_pos(
    as_iterator(handle_),
    as_image(image.handle_),
    terminal.handle_->inner,
    &col, &row
  );
  if (result == GHOSTTY_NO_VALUE) {
    return std::nullopt;
  }
  detail::throw_if_ghostty_error(result);
  return ViewportPos{col, row};
}

SourceRect PlacementIterator::source_rect(const Image& image) const {
  ensure_handle();
  std::uint32_t x = 0;
  std::uint32_t y = 0;
  std::uint32_t w = 0;
  std::uint32_t h = 0;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_placement_source_rect(
      as_iterator(handle_),
      as_image(image.handle_),
      &x, &y, &w, &h
    )
  );
  return SourceRect{x, y, w, h};
}

void PlacementIterator::ensure_handle() const {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }
}

void PlacementIterator::release() noexcept {
  if (handle_ != nullptr) {
    ghostty_kitty_graphics_placement_iterator_free(as_iterator(handle_));
    handle_ = nullptr;
  }
}

// --- Graphics ---

Image Graphics::image(std::uint32_t image_id) const {
  GhosttyKittyGraphicsImage img = ghostty_kitty_graphics_image(
    as_graphics(handle_), image_id
  );
  if (img == nullptr) {
    throw Error(ErrorCode::InvalidValue);
  }
  return Image(const_cast<void*>(static_cast<const void*>(img)));
}

PlacementIterator Graphics::placements() const {
  PlacementIterator iter;
  detail::throw_if_ghostty_error(
    ghostty_kitty_graphics_get(
      as_graphics(handle_),
      GHOSTTY_KITTY_GRAPHICS_DATA_PLACEMENT_ITERATOR,
      &iter.handle_
    )
  );
  return iter;
}

} // namespace libghostty_cpp::kitty_graphics

namespace libghostty_cpp {

kitty_graphics::Graphics Terminal::kitty_graphics() const {
  GhosttyKittyGraphics graphics = nullptr;
  detail::throw_if_ghostty_error(
    ghostty_terminal_get(
      handle_->inner, GHOSTTY_TERMINAL_DATA_KITTY_GRAPHICS, &graphics
    )
  );
  return kitty_graphics::Graphics(graphics);
}

} // namespace libghostty_cpp
