# Read PNG and write as static C array in slate.cpp
png_path = r"C:\Users\tyler\Desktop\Wizard External\silent_esp_preview.png"
slate_path = r"C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp"

with open(png_path, 'rb') as f:
    bytes_data = f.read()

# Format as static array with newlines every 16 bytes
hex_values = [f"0x{b:02X}" for b in bytes_data]
formatted_lines = []
for i in range(0, len(hex_values), 16):
    chunk = hex_values[i:i+16]
    line = "    " + ", ".join(chunk) + ","
    formatted_lines.append(line)

# Remove trailing comma from last line
formatted_lines[-1] = formatted_lines[-1].rstrip(",")

array_decl = f"static unsigned char silent_esp_preview[{len(bytes_data)}] = {{\n"
array_body = "\n".join(formatted_lines)
array_end = "\n};"

full_array = array_decl + array_body + array_end

# Read slate.cpp
with open(slate_path, 'r', encoding='utf-8') as f:
    slate_content = f.read()

# Insert after includes
includes_end = slate_content.find('}', slate_content.find('#include'))
if includes_end == -1:
    # Just insert after first few lines
    insert_pos = slate_content.find('#include')
    while slate_content[insert_pos] != '\n':
        insert_pos += 1
    insert_pos += 1
else:
    insert_pos = includes_end + 1

new_slate = slate_content[:insert_pos] + "\n" + full_array + "\n" + slate_content[insert_pos:]

with open(slate_path, 'w', encoding='utf-8') as f:
    f.write(new_slate)

print(f"Added {len(bytes_data)} byte array to slate.cpp")
