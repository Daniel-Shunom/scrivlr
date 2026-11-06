Adapted from nicbarker/clay's GLES3 renderer at revision
`e6cc36941ab2af5d81107617039d6f527a1c660b`:
https://github.com/nicbarker/clay/tree/e6cc36941ab2af5d81107617039d6f527a1c660b/renderers/GLES3

The original project uses the zlib/libpng license included in `../clay.h`.
Scrivlr supplies GLAD and desktop GLSL 330, fixes the VAO reset, reports shader
compilation failure through exceptions, and applies image tint in the shader.
The adapter submits commands individually to preserve ordering and implements
nested clipping in framebuffer pixels outside this backend.

The upstream stb loader is not used. Font baking and glyph measurement live in
`src/ui/backend.cpp`, using an embedded Noto Sans font and stb_truetype. The
current atlas covers ASCII, uses font ID 0, and substitutes unsupported bytes
with question marks. The backend has four image texture slots. Custom Clay
render commands require an additional adapter and report an error for now.
