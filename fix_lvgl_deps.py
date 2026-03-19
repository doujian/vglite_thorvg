import re

file_path = r'D:\Projects\extract_vglite_by_thorvg\src\vg_lite_tvg.cpp'

with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Step 1: Replace LVGL includes
content = re.sub(r'#include\s+"../../lv_conf_internal.h"', '/* LVGL config removed */', content)
content = re.sub(r'#include\s+"../../lvgl.h"', '/* LVGL includes removed */', content)
content = re.sub(r'#include\s+"../../libs/thorvg/thorvg.h"', '#include <thorvg.h>', content)

# Add standard includes
content = re.sub(
    r'(#include\s+<vector>)',
    r'\1\n#include <assert.h>\n#include <stdio.h>',
    content
)

# Step 2: Replace conditional compilation
content = re.sub(r'#if\s+LV_USE_DRAW_VG_LITE\s+&&\s+LV_USE_VG_LITE_THORVG', '#ifdef VG_LITE_RENDER_BACKEND', content)
content = re.sub(r'#if\s+LV_VG_LITE_THORVG_YUV_SUPPORT', '#if VG_LITE_YUV_SUPPORT', content)
content = re.sub(r'#if\s+LV_VG_LITE_THORVG_LVGL_BLEND_SUPPORT', '#if VG_LITE_LVGL_BLEND_SUPPORT', content)
content = re.sub(r'#if\s+LV_VG_LITE_THORVG_16PIXELS_ALIGN', '#if VG_LITE_16PIXELS_ALIGN', content)
content = re.sub(r'#if\s+LV_VG_LITE_THORVG_THREAD_RENDER', '#if VG_LITE_THREAD_RENDER', content)

# Step 3: Replace VLC_ macros with VG_LITE_ equivalents
content = re.sub(r'VLC_MAX_', 'VG_LITE_MAX_', content)
content = re.sub(r'LV_VG_LITE_THORVG_BUF_ADDR_ALIGN', 'VG_LITE_BUF_ADDR_ALIGN', content)

# Step 4: Replace LVGL logging macros
# Simple direct string replacements for the macro definitions
content = content.replace(
    'LV_LOG_ERROR("Executed \'" #FUNC "\' error: %d", (int)res);',
    'fprintf(stderr, "VG_LITE: Executed \'" #FUNC "\' error: %d\\n", (int)res);'
)
content = content.replace(
    'LV_LOG_USER("Executed \'" #FUNC "\' error: %d", (int)res);',
    'fprintf(stderr, "VG_LITE: Executed \'" #FUNC "\' error: %d\\n", (int)res);'
)

# Generic replacements for other LV_LOG calls
# These use regex to match format strings
def replace_log_error(m):
    format_str = m.group(1)
    args = m.group(3) if m.lastindex >= 3 and m.group(3) else None
    if args:
        return f'fprintf(stderr, "VG_LITE: {format_str}\\n", {args}'
    else:
        return f'fprintf(stderr, "VG_LITE: {format_str}\\n"'

def replace_log_warn(m):
    format_str = m.group(1)
    args = m.group(3) if m.lastindex >= 3 and m.group(3) else None
    if args:
        return f'fprintf(stderr, "VG_LITE WARN: {format_str}\\n", {args}'
    else:
        return f'fprintf(stderr, "VG_LITE WARN: {format_str}\\n"'

def replace_log_user(m):
    format_str = m.group(1)
    args = m.group(3) if m.lastindex >= 3 and m.group(3) else None
    if args:
        return f'printf("VG_LITE: {format_str}\\n", {args}'
    else:
        return f'printf("VG_LITE: {format_str}\\n"'

# Apply replacements - note: these patterns match strings NOT containing single quotes (those are handled above)
content = re.sub(r'LV_LOG_ERROR\("([^"\']*)"(\s*\))', replace_log_error, content)
content = re.sub(r'LV_LOG_ERROR\("([^"\']*)",\s*([^)]+)\)', r'fprintf(stderr, "VG_LITE: \1\\n", \2)', content)
content = re.sub(r'LV_LOG_WARN\("([^"\']*)"\)', r'fprintf(stderr, "VG_LITE WARN: \1\\n")', content)
content = re.sub(r'LV_LOG_WARN\("([^"\']*)",\s*([^)]+)\)', r'fprintf(stderr, "VG_LITE WARN: \1\\n", \2)', content)
content = re.sub(r'LV_LOG_USER\("([^"\']*)"\)', r'printf("VG_LITE: \1\\n")', content)
content = re.sub(r'LV_LOG_USER\("([^"\']*)",\s*([^)]+)\)', r'printf("VG_LITE: \1\\n", \2)', content)
content = re.sub(r'LV_LOG_TRACE\([^)]*\);', '/* trace removed */;', content)

# Step 5: Replace LVGL assertion macros
content = re.sub(r'LV_ASSERT\(', 'assert(', content)
content = re.sub(r'LV_ASSERT_NULL\(', 'assert(', content)
content = re.sub(r'LV_ASSERT_MALLOC\(', 'assert(', content)

# Step 6: Replace LVGL utility macros
content = re.sub(r'LV_UNUSED\(([^)]+)\);', r'(void)\1;', content)

# Step 7: Replace LVGL memory functions
# lv_malloc_zeroed(size) -> calloc(1, size) (lv_malloc_zeroed takes 1 arg, calloc takes 2)
content = re.sub(r'\blv_malloc_zeroed\(([^)]+)\)', r'calloc(1, \1)', content)
content = re.sub(r'\blv_malloc\b', 'malloc', content)
content = re.sub(r'\blv_free\b', 'free', content)
content = re.sub(r'lv_memzero\(([^,]+),\s*([^)]+)\)', r'memset(\1, 0, \2)', content)
content = re.sub(r'\blv_memcpy\b', 'memcpy', content)
content = re.sub(r'\blv_memcmp\b', 'memcmp', content)
content = re.sub(r'\blv_strlen\b', 'strlen', content)
content = re.sub(r'\blv_snprintf\b', 'snprintf', content)

# Step 8: Replace remaining LV_VG_LITE_THORVG_ config macros
content = re.sub(r'LV_VG_LITE_THORVG_LINEAR_GRADIENT_EXT_SUPPORT', 'VG_LITE_LINEAR_GRADIENT_EXT_SUPPORT', content)

# Step 9: Fix the closing conditional
content = re.sub(
    r'#endif\s*/\*\s*LV_USE_DRAW_VG_LITE\s*&&\s*LV_USE_VG_LITE_THORVG\s*\*/',
    '#endif /* VG_LITE_RENDER_BACKEND */',
    content
)

with open(file_path, 'w', encoding='utf-8') as f:
    f.write(content)

# Verify no remaining LV_ patterns
with open(file_path, 'r', encoding='utf-8') as f:
    new_content = f.read()

# Check for remaining LV_ patterns (excluding VG_LITE_)
remaining = re.findall(r'\bLV_[A-Z_]+\b', new_content)
remaining += re.findall(r'\blv_[a-z_]+\b', new_content)

if remaining:
    print(f"Warning: Found remaining LVGL patterns: {set(remaining)}")
else:
    print("LVGL dependencies removed successfully!")

print(f"File size: {len(new_content)} bytes")
