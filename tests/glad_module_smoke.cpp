import glad;

static_assert(GL_COLOR_BUFFER_BIT == 0x00004000);
static_assert(sizeof(GLuint) == sizeof(unsigned int));

int main() {
    // Verify the imported alias refers to the C loader's actual storage.
    if (&glClear != &glad_glClear) {
        return 1;
    }
    // An unavailable context must fail gracefully; no GPU is required.
    GLADloadproc unavailable = +[](const char*) -> void* { return nullptr; };
    return gladLoadGLLoader(unavailable) == 0 ? 0 : 2;
}
