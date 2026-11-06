#include <GLFW/glfw3.h>
#include <cassert>

import scrivlr;
import glad;

#define FAIL_EXIT 1
#define SUCC_EXIT 0

void* callbackFunction(const char* addr) {
    return reinterpret_cast<void*>(glfwGetProcAddress(addr));
}

void renderFrame(GLFWwindow* window) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);
}

void frameBufferCallback(GLFWwindow* window, int width, int height) {
    if (!window) {
        return;
    }

    glViewport(0, 0, width, height);
    renderFrame(window);
}

void windowRefreshCallback(GLFWwindow* window) {
    renderFrame(window);
}

int main(void) {
    if (!glfwInit()) {
        return FAIL_EXIT;
    };

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 800, "scrivlr", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return FAIL_EXIT;
    }

    glfwMakeContextCurrent(window);

    if (gladLoadGLLoader(&callbackFunction) == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return FAIL_EXIT;
    };

    glfwSetFramebufferSizeCallback(window, &frameBufferCallback);
    glfwSetWindowRefreshCallback(window, &windowRefreshCallback);

    while (!glfwWindowShouldClose(window)) {
        renderFrame(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return SUCC_EXIT;
}
