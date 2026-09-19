"""Generates Atom AZSL shaders from the NoesisGUI SDK reference HLSL at configure time.

The SDK's shader source is not redistributable, so outputs go to a gitignored folder.
"""
import argparse
import json
import os
import re
import sys

SDK_SHADER_DIR = ("Src", "Packages", "Render", "D3D12RenderDevice", "Src")
TEXTURES = ("pattern", "ramps", "image", "glyphs", "shadow")

# Vertex shader permutations (Noesis::Shader::Vertex order) and their defines.
VERTEX_SHADERS = {
    "Pos": [],
    "PosColor": ["HAS_COLOR"],
    "PosTex0": ["HAS_UV0"],
    "PosTex0Rect": ["HAS_UV0", "HAS_RECT"],
    "PosTex0RectTile": ["HAS_UV0", "HAS_RECT", "HAS_TILE"],
    "PosColorCoverage": ["HAS_COLOR", "HAS_COVERAGE"],
    "PosTex0Coverage": ["HAS_UV0", "HAS_COVERAGE"],
    "PosTex0CoverageRect": ["HAS_UV0", "HAS_COVERAGE", "HAS_RECT"],
    "PosTex0CoverageRectTile": ["HAS_UV0", "HAS_COVERAGE", "HAS_RECT", "HAS_TILE"],
    "PosColorTex1_SDF": ["HAS_COLOR", "HAS_UV1", "SDF"],
    "PosTex0Tex1_SDF": ["HAS_UV0", "HAS_UV1", "SDF"],
    "PosTex0Tex1Rect_SDF": ["HAS_UV0", "HAS_UV1", "HAS_RECT", "SDF"],
    "PosTex0Tex1RectTile_SDF": ["HAS_UV0", "HAS_UV1", "HAS_RECT", "HAS_TILE", "SDF"],
    "PosColorTex1": ["HAS_COLOR", "HAS_UV1"],
    "PosTex0Tex1": ["HAS_UV0", "HAS_UV1"],
    "PosTex0Tex1Rect": ["HAS_UV0", "HAS_UV1", "HAS_RECT"],
    "PosTex0Tex1RectTile": ["HAS_UV0", "HAS_UV1", "HAS_RECT", "HAS_TILE"],
    "PosColorTex0Tex1": ["HAS_COLOR", "HAS_UV0", "HAS_UV1"],
    "PosTex0Tex1_Downsample": ["HAS_UV0", "HAS_UV1", "DOWNSAMPLE"],
    "PosColorTex1Rect": ["HAS_COLOR", "HAS_UV1", "HAS_RECT"],
    "PosColorTex0RectImagePos": ["HAS_COLOR", "HAS_UV0", "HAS_RECT", "HAS_IMAGE_POSITION"],
}


def _paints(effect, vs_suffix_by_paint):
    out = {}
    patterns = [("Solid", ["PAINT_SOLID"]), ("Linear", ["PAINT_LINEAR"]), ("Radial", ["PAINT_RADIAL"]),
                ("Pattern", ["PAINT_PATTERN"]), ("Pattern_Clamp", ["PAINT_PATTERN", "CLAMP_PATTERN"]),
                ("Pattern_Repeat", ["PAINT_PATTERN", "REPEAT_PATTERN"]),
                ("Pattern_MirrorU", ["PAINT_PATTERN", "MIRRORU_PATTERN"]),
                ("Pattern_MirrorV", ["PAINT_PATTERN", "MIRRORV_PATTERN"]),
                ("Pattern_Mirror", ["PAINT_PATTERN", "MIRROR_PATTERN"])]
    for suffix, defines in patterns:
        out[suffix] = ([effect] + defines, vs_suffix_by_paint(suffix))
    return out


def _vs(solid, tex, rect, tile):
    return lambda s: solid if s == "Solid" else rect if s == "Pattern_Clamp" else tile if s.startswith("Pattern_") else tex


# Pixel shader name -> (defines, vertex shader). Mirrors Noesis::Shader::Enum and VertexForShader[].
# SDF_LCD_* (needs dual-source blending) and Custom_Effect (M5) are intentionally absent.
SHADERS = {
    "RGBA": (["EFFECT_RGBA"], "Pos"),
    "Mask": (["EFFECT_MASK"], "Pos"),
    "Clear": (["EFFECT_CLEAR"], "Pos"),
}
for prefix, effect, vs in (
    ("Path_", "EFFECT_PATH", _vs("PosColor", "PosTex0", "PosTex0Rect", "PosTex0RectTile")),
    ("Path_AA_", "EFFECT_PATH_AA", _vs("PosColorCoverage", "PosTex0Coverage", "PosTex0CoverageRect", "PosTex0CoverageRectTile")),
    ("SDF_", "EFFECT_SDF", _vs("PosColorTex1_SDF", "PosTex0Tex1_SDF", "PosTex0Tex1Rect_SDF", "PosTex0Tex1RectTile_SDF")),
    ("Opacity_", "EFFECT_OPACITY", _vs("PosColorTex1", "PosTex0Tex1", "PosTex0Tex1Rect", "PosTex0Tex1RectTile")),
):
    for suffix, entry in _paints(effect, vs).items():
        SHADERS[prefix + suffix] = entry
SHADERS["Upsample"] = (["EFFECT_UPSAMPLE"], "PosColorTex0Tex1")
SHADERS["Downsample"] = (["EFFECT_DOWNSAMPLE"], "PosTex0Tex1_Downsample")
SHADERS["Shadow"] = (["EFFECT_SHADOW", "PAINT_SOLID"], "PosColorTex1Rect")
SHADERS["Blur"] = (["EFFECT_BLUR", "PAINT_SOLID"], "PosColorTex1")

_DIRECTIVE = re.compile(r"^\s*#\s*(if|elif|else|endif|define|pragma)\b(.*)$")


def _eval(expr, defines):
    expr = expr.split("//")[0]
    py = expr.replace("||", " or ").replace("&&", " and ")
    py = re.sub(r"!(?!=)", " not ", py)
    py = re.sub(r"\b[A-Za-z_]\w*\b",
                lambda m: m.group(0) if m.group(0) in ("or", "and", "not") else ("1" if defines.get(m.group(0), "0") != "0" else "0"),
                py)
    return bool(eval(py, {"__builtins__": {}}))


def preprocess(text, defines):
    defines = dict(defines)
    out = []
    stack = []  # (active, taken)
    for line in text.splitlines():
        m = _DIRECTIVE.match(line)
        active = all(a for a, _ in stack)
        if not m:
            if active:
                out.append(line)
            continue
        kind, rest = m.group(1), m.group(2).strip()
        if kind == "if":
            value = active and _eval(rest, defines)
            stack.append((value, value))
        elif kind == "elif":
            _, taken = stack.pop()
            parent = all(a for a, _ in stack)
            value = parent and not taken and _eval(rest, defines)
            stack.append((value, taken or value))
        elif kind == "else":
            _, taken = stack.pop()
            parent = all(a for a, _ in stack)
            stack.append((parent and not taken, True))
        elif kind == "endif":
            stack.pop()
        elif kind == "define" and active:
            parts = rest.split(None, 1)
            defines[parts[0]] = parts[1] if len(parts) > 1 else "1"
            if len(parts) > 1 and parts[1] != "1":
                out.append(line)
    return "\n".join(out) + "\n"


_CBUFFER = re.compile(r"cbuffer\s+\w+\s*:\s*register\(\s*b(\d)\s*\)\s*\{(.*?)\}\s*;?", re.S)
_RESOURCE = re.compile(r"^\s*(Texture2D|SamplerState)\s+\w+\s*:\s*register\(\s*[ts]\d\s*\)\s*;\s*$", re.M)
_MEMBER = re.compile(r"(\w+)\s*(\[\d+\])?\s*;")


def _extract(stage, text):
    buffers = []
    def take(m):
        body = m.group(2).strip()
        if body:
            body = re.sub(r"\bfloat4x4\b", "column_major float4x4", body)
            buffers.append(("%s%s" % (stage, m.group(1)), body))
        return ""
    text = _CBUFFER.sub(take, text)
    text = _RESOURCE.sub("", text)
    text = re.sub(r"^\s*float4\s+main_brush\s*\(\s*float2\s+\w+\s*\)\s*;\s*$", "", text, flags=re.M)
    text = re.sub(r"^\s*#\s*define\s+HAS_\w+.*$", "", text, flags=re.M)
    upper = stage.upper()
    text = re.sub(r"\bIn\b", upper + "In", text)
    text = re.sub(r"\bOut\b", upper + "Out", text)
    text = re.sub(r"\bmain\b", upper + "Main", text)
    text = re.sub(r"\bSV_POSITION\b", "SV_Position", text)
    text = re.sub(r"\bSV_TARGET(\d*)\b", r"SV_Target\1", text)
    if stage == "vs":
        text = _return_vertex_output(text)
    return text, buffers


def _return_vertex_output(text):
    # Atom's shader builder treats entry-point out-parameters as inputs, so every interpolant semantic
    # appears twice in the input contract. Return the output struct instead.
    signature = re.compile(r"void\s+VSMain\s*\(\s*in\s+VSIn\s+i\s*,\s*out\s+VSOut\s+o\s*\)\s*\{")
    if not signature.search(text):
        raise ValueError("vertex entry point signature not recognized")
    text = signature.sub("VSOut VSMain(VSIn i)\n{\n    VSOut o;", text)
    body_end = text.rstrip().rfind("}")
    return text[:body_end] + "    return o;\n}\n"


def to_azsl(vs_text, ps_text):
    vs, vs_buffers = _extract("vs", vs_text)
    ps, ps_buffers = _extract("ps", ps_text)
    buffers = vs_buffers + ps_buffers

    replacements = {}
    owners = {}  # identifier -> owning buffer/texture key, to catch cross-buffer name collisions

    def _add(name, target, owner):
        if name in owners and owners[name] != owner:
            raise ValueError("shader identifier '%s' is declared in both %s and %s" % (name, owners[name], owner))
        replacements[name] = target
        owners[name] = owner

    for key, body in buffers:
        for name, _ in _MEMBER.findall(body):
            _add(name, "NoesisDrawSrg::m_%s.%s" % (key, name), key)
    for name in TEXTURES:
        _add(name, "NoesisDrawSrg::m_%s" % name, "texture")
        _add(name + "Sampler", "NoesisDrawSrg::m_%sSampler" % name, "texture")

    def rewrite(code):
        if not replacements:
            return code
        pattern = re.compile(r"(?<![.\w])(%s)\b" % "|".join(sorted(map(re.escape, replacements), key=len, reverse=True)))
        return pattern.sub(lambda m: replacements[m.group(1)], code)

    lines = ["// Generated by Tools/generate_shaders.py from the NoesisGUI SDK. Do not edit or commit.",
             "#include <Atom/Features/SrgSemantics.azsli>", ""]
    for key, body in buffers:
        lines.append("struct Noesis%s\n{\n    %s\n};\n" % (key.upper(), body))
    lines.append("ShaderResourceGroup NoesisDrawSrg : SRG_PerDraw\n{")
    for key, _ in buffers:
        lines.append("    Noesis%s m_%s;" % (key.upper(), key))
    for name in TEXTURES:
        lines.append("    Texture2D m_%s;" % name)
        lines.append("    Sampler m_%sSampler;" % name)
    lines.append("}\n")
    lines.append(rewrite(vs))
    lines.append(rewrite(ps))
    azsl = "\n".join(lines)
    return re.sub(r"\bhalf([234]?)\b", r"float\1", azsl)


def shader_json(name):
    return json.dumps({
        "Source": name + ".azsl",
        "DrawList": "noesisgui",
        "RasterState": {"CullMode": "None"},
        "DepthStencilState": {"Depth": {"Enable": False}},
        "ProgramSettings": {"EntryPoints": [{"name": "VSMain", "type": "Vertex"}, {"name": "PSMain", "type": "Fragment"}]},
    }, indent=4)


def generate(sdk_root, out_dir):
    src = os.path.join(sdk_root, *SDK_SHADER_DIR)
    with open(os.path.join(src, "ShaderVS.hlsl")) as f:
        vs_source = f.read()
    with open(os.path.join(src, "ShaderPS.hlsl")) as f:
        ps_source = f.read()
    os.makedirs(out_dir, exist_ok=True)
    for name, (ps_defines, vs_name) in SHADERS.items():
        vs = preprocess(vs_source, {d: "1" for d in VERTEX_SHADERS[vs_name]})
        ps = preprocess(ps_source, {d: "1" for d in ps_defines})
        _write_if_changed(os.path.join(out_dir, name + ".azsl"), to_azsl(vs, ps))
        _write_if_changed(os.path.join(out_dir, name + ".shader"), shader_json(name))


def _write_if_changed(path, content):
    # Rewriting identical files would make the Asset Processor recompile every shader on each configure.
    if os.path.isfile(path):
        with open(path) as f:
            if f.read() == content:
                return
    with open(path, "w", newline="\n") as f:
        f.write(content)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sdk", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()
    generate(args.sdk, args.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
