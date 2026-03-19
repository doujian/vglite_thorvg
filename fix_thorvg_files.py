import os
import re

# Directory containing ThorVG source files
thorvg_dir = r'D:\Projects\extract_vglite_by_thorvg\third_party\thorvg'

# Pattern to find and remove LVGL conditionals
patterns = [
    # Remove LVGL config include
    (r'#include\s+"../../lv_conf_internal.h"\s*\n', ''),
    
    # Remove LV_USE_THORVG_INTERNAL conditional start (and any comments before it)
    (r'#if\s+LV_USE_THORVG_INTERNAL\s*\n', ''),
    
    # Remove closing #endif for LV_USE_THORVG_INTERNAL (at end of file)
    (r'\n#endif\s*/\*\s*LV_USE_THORVG_INTERNAL\s*\*/\s*$', '\n'),
    (r'\n#endif\s*//\s*LV_USE_THORVG_INTERNAL\s*$', '\n'),
    (r'\n#endif\s*\/\*\s*LV_USE_THORVG_INTERNAL\s*\*\/\s*$', '\n'),
    
    # Remove other LVGL config references
    (r'#if\s+LV_USE_THORVG\s*\n', '#if 0 // LVGL disabled\n'),
]

def fix_thorvg_file(filepath):
    """Fix a single ThorVG source file."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except:
        print(f"  Skipping (cannot read): {filepath}")
        return False
    
    original = content
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content)
    
    # Check if file was modified
    if content != original:
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  Fixed: {os.path.basename(filepath)}")
            return True
        except Exception as e:
            print(f"  Error writing {filepath}: {e}")
            return False
    else:
        return False

def main():
    print("Fixing ThorVG source files...")
    
    # Process all .cpp files
    fixed_count = 0
    for filename in os.listdir(thorvg_dir):
        if filename.endswith('.cpp'):
            filepath = os.path.join(thorvg_dir, filename)
            if fix_thorvg_file(filepath):
                fixed_count += 1
    
    print(f"\nFixed {fixed_count} files.")

if __name__ == '__main__':
    main()
