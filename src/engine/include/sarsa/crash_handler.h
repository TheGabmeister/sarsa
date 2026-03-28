#pragma once

namespace sarsa {

// Installs platform-specific crash handlers.
// On Windows: sets an unhandled exception filter that writes a minidump
// and dumps recent log messages to a crash log file.
void install_crash_handler();

} // namespace sarsa
