import sys
data = open(sys.argv[1], "rb").read()
with open(sys.argv[2], "w") as f:
    f.write("#pragma once\n#include <cstdint>\n#include <cstddef>\n\nnamespace EmbeddedAssets {\n    inline constexpr uint8_t DEFAULT_FONT_TTF[] = {\n")
    for i in range(0, len(data), 16):
        chunk = ", ".join(f"0x{b:02X}" for b in data[i:i+16])
        f.write(f"        {chunk}{',' if i + 16 < len(data) else ''}\n")
    f.write("    };\n    inline constexpr size_t DEFAULT_FONT_TTF_SIZE = sizeof(DEFAULT_FONT_TTF);\n}\n")