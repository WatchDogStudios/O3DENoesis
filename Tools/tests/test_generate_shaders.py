import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import generate_shaders as gen


class PreprocessTests(unittest.TestCase):
    def test_if_elif_else(self):
        src = "#if A\na\n#elif B\nb\n#else\nc\n#endif\n"
        self.assertEqual(gen.preprocess(src, {"B": "1"}).strip(), "b")
        self.assertEqual(gen.preprocess(src, {}).strip(), "c")

    def test_or_and_nested_and_define(self):
        src = "#if X || Y\n#define Z 1\n#endif\n#if Z && !W\nz\n#endif\n"
        out = gen.preprocess(src, {"Y": "1"})
        self.assertIn("z", out)
        self.assertNotIn("#if", out)

    def test_keeps_value_defines_and_drops_pragma(self):
        out = gen.preprocess("#define K 7.5\n#pragma warning (disable : 1)\nk\n", {})
        self.assertIn("#define K 7.5", out)
        self.assertNotIn("#pragma", out)

    def test_literal_if_1(self):
        self.assertEqual(gen.preprocess("#if 1\na\n#else\nb\n#endif\n", {}).strip(), "a")


class TransformTests(unittest.TestCase):
    VS = (
        "struct In { float2 position: POSITION; };\n"
        "struct Out { float4 position: SV_POSITION; };\n"
        "cbuffer Buffer0: register(b0)\n{\n    float4x4 projectionMtx;\n}\n"
        "cbuffer Buffer1: register(b1)\n{\n}\n"
        "void main(in In i, out Out o) { o.position = mul(float4(i.position, 0, 1), projectionMtx); }\n"
    )
    PS = (
        "Texture2D pattern: register(t0);\nSamplerState patternSampler: register(s0);\n"
        "cbuffer Buffer0: register(b0)\n{\n    float opacity;\n}\n"
        "struct In { float4 position: SV_POSITION; half2 uv: TEXCOORD0; };\n"
        "struct Out { half4 color: SV_TARGET0; };\n"
        "float4 main_brush(float2 uv);\n"
        "Out main(in In i) { Out o; half opacity_ = (half)opacity; o.color = pattern.Sample(patternSampler, i.uv) * opacity_; return o; }\n"
    )

    def setUp(self):
        self.azsl = gen.to_azsl(self.VS, self.PS)

    def test_srg_contains_nonempty_buffers_and_all_textures(self):
        self.assertIn("ShaderResourceGroup NoesisDrawSrg : SRG_PerDraw", self.azsl)
        self.assertIn("NoesisVS0 m_vs0;", self.azsl)
        self.assertIn("NoesisPS0 m_ps0;", self.azsl)
        self.assertNotIn("m_vs1", self.azsl)
        for name in gen.TEXTURES:
            self.assertIn("Texture2D m_%s;" % name, self.azsl)
            self.assertIn("Sampler m_%sSampler;" % name, self.azsl)

    def test_matrices_are_column_major(self):
        self.assertIn("column_major float4x4 projectionMtx;", self.azsl)

    def test_references_rewritten_but_not_members_or_suffixes(self):
        self.assertIn("NoesisDrawSrg::m_vs0.projectionMtx", self.azsl)
        self.assertIn("(float)NoesisDrawSrg::m_ps0.opacity", self.azsl)
        self.assertIn("float opacity_", self.azsl)
        self.assertIn("NoesisDrawSrg::m_pattern.Sample(NoesisDrawSrg::m_patternSampler", self.azsl)

    def test_no_hlsl_only_constructs_remain(self):
        for token in ("cbuffer", "register(", "SamplerState", "half", "main_brush"):
            self.assertNotIn(token, self.azsl)

    def test_entry_points_and_structs_renamed(self):
        self.assertIn("PSOut PSMain(in PSIn i)", self.azsl)

    def test_vertex_outputs_are_returned_not_out_params(self):
        # Atom's shader builder reads out-parameters as inputs, duplicating every interpolant semantic.
        self.assertIn("VSOut VSMain(VSIn i)", self.azsl)
        self.assertNotIn("out VSOut", self.azsl)
        self.assertRegex(self.azsl, r"VSOut o;[\s\S]*return o;\s*\}")

    def test_system_value_semantics_use_atom_casing(self):
        self.assertIn("SV_Position", self.azsl)
        self.assertIn("SV_Target0", self.azsl)
        self.assertNotIn("SV_POSITION", self.azsl)
        self.assertNotIn("SV_TARGET", self.azsl)


class TableTests(unittest.TestCase):
    def test_generated_set(self):
        self.assertEqual(len(gen.SHADERS), 43)
        self.assertNotIn("SDF_LCD_Solid", gen.SHADERS)
        self.assertEqual(gen.SHADERS["SDF_Solid"], (["EFFECT_SDF", "PAINT_SOLID"], "PosColorTex1_SDF"))

    def test_writes_files(self):
        with tempfile.TemporaryDirectory() as sdk, tempfile.TemporaryDirectory() as out:
            src = os.path.join(sdk, *gen.SDK_SHADER_DIR)
            os.makedirs(src)
            with open(os.path.join(src, "ShaderVS.hlsl"), "w") as f:
                f.write(TransformTests.VS)
            with open(os.path.join(src, "ShaderPS.hlsl"), "w") as f:
                f.write(TransformTests.PS)
            gen.generate(sdk, out)
            self.assertTrue(os.path.isfile(os.path.join(out, "Path_Solid.azsl")))
            with open(os.path.join(out, "Path_Solid.shader")) as f:
                shader = f.read()
            self.assertIn('"Source": "Path_Solid.azsl"', shader)
            self.assertIn('"name": "VSMain"', shader)


class CollisionTests(unittest.TestCase):
    def test_member_name_shared_across_vs_and_ps_buffers_raises(self):
        vs = (
            "struct In { float2 position: POSITION; };\n"
            "struct Out { float4 position: SV_POSITION; };\n"
            "cbuffer Buffer0: register(b0)\n{\n    float4x4 opacity;\n}\n"
            "void main(in In i, out Out o) { o.position = mul(float4(i.position, 0, 1), opacity); }\n"
        )
        ps = (
            "struct In { float4 position: SV_POSITION; };\n"
            "struct Out { half4 color: SV_TARGET0; };\n"
            "cbuffer Buffer0: register(b0)\n{\n    float opacity;\n}\n"
            "Out main(in In i) { Out o; o.color = (half4)opacity; return o; }\n"
        )
        with self.assertRaises(ValueError):
            gen.to_azsl(vs, ps)


if __name__ == "__main__":
    unittest.main()
