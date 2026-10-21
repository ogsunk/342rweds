import sys

# Read PNG file
png_path = r"C:\Users\tyler\Desktop\Wizard External\src\slate\resources\sunketsware_animated_spritesheet.png"
output_path = r"C:\Users\tyler\Desktop\Wizard External\sunketsware_animated_array.txt"

with open(png_path, 'rb') as f:
    bytes_data = f.read()

hex_list = [f"0x{b:02X}" for b in bytes_data]
header = f"inline unsigned char logobinary[{len(bytes_data)}] = {{"
byte_string = ", ".join(hex_list)
footer = "};"

with open(output_path, 'w') as f:
    f.write(header + byte_string + footer)

print(f"Converted {len(bytes_data)} bytes to C array format")
