#pragma once

#include <optional>

struct GLFWwindow;

namespace sarsa {

struct WindowConfig {
    int width = 1280;
    int height = 720;
    const char* title = "Sarsa Engine";
};

class Window {
public:
    static std::optional<Window> create(WindowConfig config = {});
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    [[nodiscard]] bool should_close() const;
    void poll_events();

    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] bool is_minimized() const;
    [[nodiscard]] GLFWwindow* handle() const { return m_window; }

private:
    explicit Window(GLFWwindow* window);
    GLFWwindow* m_window = nullptr;
};

} // namespace sarsa
