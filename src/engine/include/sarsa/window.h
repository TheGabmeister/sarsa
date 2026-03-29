#pragma once

struct GLFWwindow;

namespace sarsa {

struct WindowConfig {
    int width = 1280;
    int height = 720;
    const char* title = "Sarsa Engine";
};

class Window {
public:
    explicit Window(WindowConfig config = {});
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    [[nodiscard]] bool should_close() const;
    void poll_events();

    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] bool is_minimized() const;
    [[nodiscard]] GLFWwindow* handle() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
};

} // namespace sarsa
