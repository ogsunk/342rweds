import sys

with open(r'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\sunketsware.png', 'rb') as f:
    data = f.read()

print(f'unsigned char sunketsware[{len(data)}] = {{')
print(', '.join(f'0x{b:02X}' for b in data))
print('};')
