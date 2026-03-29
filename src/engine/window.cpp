#include <sarsa/window.h>
#include <sarsa/log.h>

#include <GLFW/glfw3.h>

namespace {

int s_glfw_ref_count = 0;

void glfw_error_callback(int error_code, const char* description) {
    SR_LOG_ENGINE(error, "GLFW error {}: {}", error_code, description);
}

void release_glfw() {
    --s_glfw_ref_count;
    if (s_glfw_ref_count == 0) {
        glfwTerminate();
    }
}

} // namespace

namespace sarsa {

Window::Window(GLFWwindow* window) : m_window(window) {}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        release_glfw();
    }
}

Window::Window(Window&& other) noexcept : m_window(other.m_window) {
    other.m_window = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (m_window) {
            glfwDestroyWindow(m_window);
            release_glfw();
        }
        m_window = other.m_window;
        other.m_window = nullptr;
    }
    return *this;
}

std::optional<Window> Window::create(WindowConfig config) {
    glfwSetErrorCallback(glfw_error_callback);

    if (s_glfw_ref_count == 0) {
        if (glfwInit() != GLFW_TRUE) {
            SR_LOG_ENGINE(error, "Failed to initialize GLFW");
            return std::nullopt;
        }
    }
    ++s_glfw_ref_count;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* handle = glfwCreateWindow(
        config.width, config.height, config.title, nullptr, nullptr);
    if (!handle) {
        SR_LOG_ENGINE(error, "Failed to create GLFW window");
        release_glfw();
        return std::nullopt;
    }

    SR_LOG_ENGINE(info, "Window created: {}x{}", config.width, config.height);
    return Window(handle);
}

bool Window::should_close() const {
    return glfwWindowShouldClose(m_window) != 0;
}

void Window::poll_events() {
    glfwPollEvents();
}

int Window::width() const {
    int w = 0;
    int h = 0;
    glfwGetWindowSize(m_window, &w, &h);
    return w;
}

int Window::height() const {
    int w = 0;
    int h = 0;
    glfwGetWindowSize(m_window, &w, &h);
    return h;
}

bool Window::is_minimized() const {
    return width() == 0 || height() == 0;
}

} // namespace sarsa
