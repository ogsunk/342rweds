import re

# Read the header file
with open(r'C:\Users\tyler\Documents\future\future\SunketsWare-FiveM\SunketsWare-FiveM\src\Ext\Includes\CustomWidgets\silent_esp_texture.h', 'r') as f:
    content = f.read()

# Extract the byte array
match = re.search(r'inline unsigned char silent_esp\[\] = \{(.*?)\};', content, re.DOTALL)
if not match:
    print("Could not find silent_esp array")
    exit(1)

hex_data = match.group(1)

# Parse hex values
bytes_list = []
for line in hex_data.split('\n'):
    line = line.strip()
    if not line or line.startswith('//'):
        continue
    # Remove trailing comma if present
    line = line.rstrip(',')
    # Split by comma and parse each hex value
    values = [v.strip() for v in line.split(',')]
    for val in values:
        if val:
            try:
                bytes_list.append(int(val, 16))
            except ValueError:
                print(f"Skipping invalid value: {val}")

print(f"Extracted {len(bytes_list)} bytes")

# Write to PNG file
output_path = r'C:\Users\tyler\Desktop\Wizard External\silent_esp_preview.png'
with open(output_path, 'wb') as f:
    f.write(bytes(bytes_list))

print(f"Saved to: {output_path}")

# Check PNG signature
if bytes_list[:8] == [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]:
    print("Valid PNG signature detected")
else:
    print("WARNING: Invalid PNG signature")
    print(f"First 8 bytes: {bytes_list[:8]}")
