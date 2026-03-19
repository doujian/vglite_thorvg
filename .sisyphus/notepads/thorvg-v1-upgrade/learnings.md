# ThorVG v1.0 Upgrade Learnings

## Task: Update CompositeMethod → MaskMethod

### Summary
Updated all `CompositeMethod` related code in `src/vg_lite_tvg.cpp` to use ThorVG v1.0's new API.

### Changes Made

**4 locations updated:**

1. **Line ~688** (blit_rect function):
   - Old: `picture->composite(std::move(shape), CompositeMethod::ClipPath)`
   - New: `picture->clip(shape.release())`

2. **Line ~952** (mask operation):
   - Old: `picture->composite(std::move(mask), CompositeMethod::InvAlphaMask)`
   - New: `picture->mask(mask.get(), MaskMethod::InvAlpha)`

3. **Line ~1226** (blit_rect function):
   - Old: `picture->composite(std::move(shape), CompositeMethod::ClipPath)`
   - New: `picture->clip(shape.release())`

4. **Line ~2616** (shape clip):
   - Old: `shape->composite(std::move(cilp), CompositeMethod::ClipPath)`
   - New: `shape->clip(cilp.release())`

### Key Findings

- ThorVG v1.0 removed `CompositeMethod` enum entirely
- Replaced by `MaskMethod` enum (None, Alpha, InvAlpha, Luma, InvLuma, Add, Subtract, Intersect, Difference, Lighten, Darken)
- `clip(Shape*)` method replaces `composite(..., CompositeMethod::ClipPath)`
- `mask(Paint*, MaskMethod)` method replaces `composite(..., CompositeMethod::XXXMask)`

### Verification

- Build errors related to `CompositeMethod` are now resolved
- No more "CompositeMethod is not a class or namespace" errors
- Remaining build errors are unrelated (other v1.0 API changes)