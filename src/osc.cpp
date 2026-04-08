#include "libghostty_cpp/osc.hpp"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"

#include <utility>

namespace libghostty_cpp::osc {

namespace {

CommandKind translate_kind(GhosttyOscCommandType kind) {
  switch (kind) {
    case GHOSTTY_OSC_COMMAND_INVALID:
      return CommandKind::Invalid;
    case GHOSTTY_OSC_COMMAND_CHANGE_WINDOW_TITLE:
      return CommandKind::ChangeWindowTitle;
    case GHOSTTY_OSC_COMMAND_CHANGE_WINDOW_ICON:
      return CommandKind::ChangeWindowIcon;
    case GHOSTTY_OSC_COMMAND_SEMANTIC_PROMPT:
      return CommandKind::SemanticPrompt;
    case GHOSTTY_OSC_COMMAND_CLIPBOARD_CONTENTS:
      return CommandKind::ClipboardContents;
    case GHOSTTY_OSC_COMMAND_REPORT_PWD:
      return CommandKind::ReportPwd;
    case GHOSTTY_OSC_COMMAND_MOUSE_SHAPE:
      return CommandKind::MouseShape;
    case GHOSTTY_OSC_COMMAND_COLOR_OPERATION:
      return CommandKind::ColorOperation;
    case GHOSTTY_OSC_COMMAND_KITTY_COLOR_PROTOCOL:
      return CommandKind::KittyColorProtocol;
    case GHOSTTY_OSC_COMMAND_SHOW_DESKTOP_NOTIFICATION:
      return CommandKind::ShowDesktopNotification;
    case GHOSTTY_OSC_COMMAND_HYPERLINK_START:
      return CommandKind::HyperlinkStart;
    case GHOSTTY_OSC_COMMAND_HYPERLINK_END:
      return CommandKind::HyperlinkEnd;
    case GHOSTTY_OSC_COMMAND_CONEMU_SLEEP:
      return CommandKind::ConemuSleep;
    case GHOSTTY_OSC_COMMAND_CONEMU_SHOW_MESSAGE_BOX:
      return CommandKind::ConemuShowMessageBox;
    case GHOSTTY_OSC_COMMAND_CONEMU_CHANGE_TAB_TITLE:
      return CommandKind::ConemuChangeTabTitle;
    case GHOSTTY_OSC_COMMAND_CONEMU_PROGRESS_REPORT:
      return CommandKind::ConemuProgressReport;
    case GHOSTTY_OSC_COMMAND_CONEMU_WAIT_INPUT:
      return CommandKind::ConemuWaitInput;
    case GHOSTTY_OSC_COMMAND_CONEMU_GUIMACRO:
      return CommandKind::ConemuGuiMacro;
    case GHOSTTY_OSC_COMMAND_CONEMU_RUN_PROCESS:
      return CommandKind::ConemuRunProcess;
    case GHOSTTY_OSC_COMMAND_CONEMU_OUTPUT_ENVIRONMENT_VARIABLE:
      return CommandKind::ConemuOutputEnvironmentVariable;
    case GHOSTTY_OSC_COMMAND_CONEMU_XTERM_EMULATION:
      return CommandKind::ConemuXtermEmulation;
    case GHOSTTY_OSC_COMMAND_CONEMU_COMMENT:
      return CommandKind::ConemuComment;
    case GHOSTTY_OSC_COMMAND_KITTY_TEXT_SIZING:
      return CommandKind::KittyTextSizing;
  }

  throw Error(ErrorCode::InvalidValue);
}

} // namespace

Parser::Parser() {
  GhosttyOscParser handle = nullptr;
  detail::throw_if_ghostty_error(ghostty_osc_new(nullptr, &handle));
  handle_ = handle;
}

Parser::~Parser() {
  release();
}

Parser::Parser(Parser&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}

Parser& Parser::operator=(Parser&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  return *this;
}

void Parser::reset() noexcept {
  if (handle_ != nullptr) {
    ghostty_osc_reset(static_cast<GhosttyOscParser>(handle_));
  }
}

void Parser::next_byte(std::uint8_t byte) noexcept {
  if (handle_ != nullptr) {
    ghostty_osc_next(static_cast<GhosttyOscParser>(handle_), byte);
  }
}

Command Parser::end(std::uint8_t terminator) {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  const GhosttyOscCommand command = ghostty_osc_end(
    static_cast<GhosttyOscParser>(handle_),
    terminator
  );
  const CommandKind kind = translate_kind(ghostty_osc_command_type(command));
  if (kind != CommandKind::ChangeWindowTitle) {
    return Command(kind);
  }

  const char* title = nullptr;
  if (!ghostty_osc_command_data(command, GHOSTTY_OSC_DATA_CHANGE_WINDOW_TITLE_STR, &title)
      || title == nullptr) {
    return Command(kind);
  }

  return Command(kind, title);
}

void Parser::release() noexcept {
  if (handle_ != nullptr) {
    ghostty_osc_free(static_cast<GhosttyOscParser>(handle_));
    handle_ = nullptr;
  }
}

} // namespace libghostty_cpp::osc
