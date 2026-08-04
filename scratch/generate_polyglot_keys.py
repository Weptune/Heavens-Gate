import random

random.seed(0x12345678)

def rand64():
    return random.randint(0, 0xFFFFFFFFFFFFFFFF)

with open(r"c:\Users\abhin\heavensgate\src\core\polyglot_keys.inc", "w") as f:
    for i in range(15):
        f.write("    { // Table " + str(i) + "\n        ")
        for sq in range(64):
            val = rand64()
            f.write(f"0x{val:016X}ULL" + (", " if sq < 63 else ""))
            if (sq + 1) % 4 == 0 and sq < 63:
                f.write("\n        ")
        f.write("\n    }" + (",\n" if i < 14 else "\n"))

print("polyglot_keys.inc generated successfully.")
