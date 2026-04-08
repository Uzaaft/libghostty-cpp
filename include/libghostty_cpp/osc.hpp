#pragma once

#include <cstdint>
#include <string>
#include <utility>

namespace libghostty_cpp::osc {

enum class CommandKind {
  Invalid,
  ChangeWindowTitle,
  ChangeWindowIcon,
  SemanticPrompt,
  ClipboardContents,
  ReportPwd,
  MouseShape,
  ColorOperation,
  KittyColorProtocol,
  ShowDesktopNotification,
  HyperlinkStart,
  HyperlinkEnd,
  ConemuSleep,
  ConemuShowMessageBox,
  ConemuChangeTabTitle,
  ConemuProgressReport,
  ConemuWaitInput,
  ConemuGuiMacro,
  ConemuRunProcess,
  ConemuOutputEnvironmentVariable,
  ConemuXtermEmulation,
  ConemuComment,
  KittyTextSizing,
};

class Command {
public:
  [[nodiscard]] CommandKind kind() const noexcept {
    return kind_;
  }

  [[nodiscard]] const std::string& title() const noexcept {
    return title_;
  }

private:
  explicit Command(CommandKind kind, std::string title = {})
      : kind_(kind), title_(std::move(title)) {}

  CommandKind kind_ = CommandKind::Invalid;
  std::string title_;

  friend class Parser;
};

class Parser {
public:
  Parser();
  ~Parser();

  Parser(Parser&& other) noexcept;
  Parser& operator=(Parser&& other) noexcept;

  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  void reset() noexcept;
  void next_byte(std::uint8_t byte) noexcept;
  [[nodiscard]] Command end(std::uint8_t terminator);

private:
  void release() noexcept;

  void* handle_ = nullptr;
};

} // namespace libghostty_cpp::osc
