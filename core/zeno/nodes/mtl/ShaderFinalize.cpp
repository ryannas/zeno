#include <zeno/zeno.h>
#include <zeno/extra/ShaderNode.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/ShaderObject.h>
#include <zeno/types/MaterialObject.h>
#include <zeno/utils/string.h>

namespace zeno {


struct ShaderFinalize : INode {
    virtual void apply() override {
        EmissionPass em;
        auto backend = get_param<std::string>("backend");
        if (backend == "HLSL")
            em.backend = em.HLSL;
        else if (backend == "GLSL")
            em.backend = em.GLSL;

        if (has_input("commonCode"))
            em.commonCode += get_input<StringObject>("commonCode")->get();

        auto code = em.finalizeCode({
            {3, "mat_basecolor"},
            {1, "mat_metallic"},
            {1, "mat_roughness"},
            {1, "mat_specular"},
            {3, "mat_normal"},
            {3, "mat_emission"},
            {1, "mat_zenxposure"},
        }, {
            get_input<IObject>("basecolor", std::make_shared<NumericObject>(vec3f(1.0f))),
            get_input<IObject>("metallic", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("roughness", std::make_shared<NumericObject>(float(0.4f))),
            get_input<IObject>("specular", std::make_shared<NumericObject>(float(0.5f))),
            get_input<IObject>("normal", std::make_shared<NumericObject>(vec3f(0, 0, 1))),
            get_input<IObject>("emission", std::make_shared<NumericObject>(vec3f(0))),
            get_input<IObject>("exposure", std::make_shared<NumericObject>(float(1.0f))),
        });
        auto commonCode = em.getCommonCode();

        auto mtl = std::make_shared<MaterialObject>();
        mtl->frag = std::move(code);
        mtl->common = std::move(commonCode);
        if (has_input("extensionsCode"))
            mtl->extensions = get_input<zeno::StringObject>("extensionsCode")->get();
        set_output("mtl", std::move(mtl));
    }
};

ZENDEFNODE(ShaderFinalize, {
    {
        {"vec3f", "basecolor", "1,1,1"},
        {"float", "metallic", "0.0"},
        {"float", "roughness", "0.4"},
        {"float", "specular", "0.5"},
        {"vec3f", "normal", "0,0,1"},
        {"vec3f", "emission", "0,0,0"},
        {"float", "exposure", "1.0"},
        {"string", "commonCode"},
        {"string", "extensionsCode"},
    },
    {
        {"MaterialObject", "mtl"},
    },
    {
        {"enum GLSL HLSL", "backend", "GLSL"},
    },
    {"shader"},
});


}
