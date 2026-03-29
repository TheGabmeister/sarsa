#include <sarsa/window.h>
#include <sarsa/log.h>

#include <GLFW/glfw3.h>

#include <cstdlib>

namespace {

void glfw_error_callback(int error_code, const char* description) {
    SR_LOG_ENGINE(error, "GLFW error {}: {}", error_code, description);
}

} // namespace

namespace sarsa {

Window::Window(WindowConfig config) {
    glfwSetErrorCallback(glfw_error_callback);

    if (glfwInit() != GLFW_TRUE) {
        SR_LOG_ENGINE(critical, "Failed to initialize GLFW");
        std::abort();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
    if (!m_window) {
        SR_LOG_ENGINE(critical, "Failed to create GLFW window");
        glfwTerminate();
        std::abort();
    }

    SR_LOG_ENGINE(info, "Window created: {}x{}", config.width, config.height);
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
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
