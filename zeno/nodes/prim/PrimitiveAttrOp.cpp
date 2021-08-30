#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>

namespace zeno {

template <class T, class S>
inline constexpr bool is_decay_same_v = std::is_same_v<std::decay_t<T>, std::decay_t<S>>;

struct PrimitiveFillAttr : zeno::INode {
  virtual void apply() override {
    auto prim = get_input<PrimitiveObject>("prim");
    auto const &value = get_input<NumericObject>("value")->value;
    auto attrName = std::get<std::string>(get_param("attrName"));
    auto attrType = std::get<std::string>(get_param("attrType"));
    if (std::holds_alternative<zeno::vec3f>(value)) {
        attrType = "float3";
    }
    if (!prim->has_attr(attrName)) {
        if (attrType == "float3") prim->add_attr<zeno::vec3f>(attrName);
        else if (attrType == "float") prim->add_attr<float>(attrName);
    }
    auto &arr = prim->attr(attrName);
    std::visit([](auto &arr, auto const &value) {
        if constexpr (zeno::is_vec_castable_v<decltype(arr[0]), decltype(value)>) {
            #pragma omp parallel for
            for (int i = 0; i < arr.size(); i++) {
                arr[i] = decltype(arr[i])(value);
            }
        } else {
            throw zeno::Exception("Failed to promote variant type");
        }
    }, arr, value);

    set_output("prim", get_input("prim"));
  }
};

ZENDEFNODE(PrimitiveFillAttr,
    { /* inputs: */ {
    "prim",
    "value",
    }, /* outputs: */ {
    "prim",
    }, /* params: */ {
    {"string", "attrName", "pos"},
    {"enum float float3", "attrType", ""},
    }, /* category: */ {
    "primitive",
    }});


void print_cout(float x) {
    printf("%f\n", x);
}

void print_cout(zeno::vec3f const &a) {
    printf("%f %f %f\n", a[0], a[1], a[2]);
}


struct PrimitivePrintAttr : zeno::INode {
  virtual void apply() override {
    auto prim = get_input<PrimitiveObject>("prim");
    auto attrName = std::get<std::string>(get_param("attrName"));
    std::visit([attrName](auto const &arr) {
        printf("attribute `%s`, length %zd:\n", attrName.c_str(), arr.size());
        for (int i = 0; i < arr.size(); i++) {
            print_cout(arr[i]);
        }
        if (arr.size() == 0) {
            printf("(no data)\n");
        }
        printf("\n");
    }, prim->attr(attrName));

    set_output("prim", get_input("prim"));
  }
};

ZENDEFNODE(PrimitivePrintAttr,
    { /* inputs: */ {
    "prim",
    }, /* outputs: */ {
    "prim",
    }, /* params: */ {
    {"string", "attrName", "pos"},
    }, /* category: */ {
    "primitive",
    }});


// deprecated: use PrimitiveRandomAttr instead
struct PrimitiveRandomizeAttr : zeno::INode {
  virtual void apply() override {
    auto prim = get_input<PrimitiveObject>("prim");
    auto min = std::get<float>(get_param("min"));
    auto minY = std::get<float>(get_param("minY"));
    auto minZ = std::get<float>(get_param("minZ"));
    auto max = std::get<float>(get_param("max"));
    auto maxY = std::get<float>(get_param("maxY"));
    auto maxZ = std::get<float>(get_param("maxZ"));
    auto attrName = std::get<std::string>(get_param("attrName"));
    auto &arr = prim->attr(attrName);
    std::visit([min, minY, minZ, max, maxY, maxZ](auto &arr) {
        for (int i = 0; i < arr.size(); i++) {
            if constexpr (is_decay_same_v<decltype(arr[i]), zeno::vec3f>) {
                zeno::vec3f f(drand48(), drand48(), drand48());
                zeno::vec3f a(min, minY, minZ);
                zeno::vec3f b(max, maxY, maxZ);
                arr[i] = zeno::mix(a, b, f);
            } else {
                arr[i] = zeno::mix(min, max, (float)drand48());
            }
        }
    }, arr);

    set_output("prim", get_input("prim"));
  }
};

ZENDEFNODE(PrimitiveRandomizeAttr,
    { /* inputs: */ {
    "prim",
    }, /* outputs: */ {
    "prim",
    }, /* params: */ {
    {"string", "attrName", "pos"},
    {"float", "min", "-1"},
    {"float", "minY", "-1"},
    {"float", "minZ", "-1"},
    {"float", "max", "1"},
    {"float", "maxY", "1"},
    {"float", "maxZ", "1"},
    }, /* category: */ {
    "primitive",
    }});


struct PrimitiveRandomAttr : zeno::INode {
  virtual void apply() override {
    auto prim = has_input("prim") ?
        get_input<PrimitiveObject>("prim") :
        std::make_shared<zeno::PrimitiveObject>();
    auto min = get_input<zeno::NumericObject>("min");
    auto max = get_input<zeno::NumericObject>("max");
    auto attrName = std::get<std::string>(get_param("attrName"));
    auto attrType = std::get<std::string>(get_param("attrName"));
    if (!prim->has_attr(attrName)) {
        if (attrType == "float3") prim->add_attr<zeno::vec3f>(attrName);
        else if (attrType == "float") prim->add_attr<float>(attrName);
    }
    auto &arr = prim->attr(attrName);
    std::visit([&](auto &arr) {
        for (int i = 0; i < arr.size(); i++) {
            if constexpr (is_decay_same_v<decltype(arr[i]), zeno::vec3f>) {
                zeno::vec3f f(drand48(), drand48(), drand48());
                auto a = min->get<zeno::vec3f>();
                auto b = min->get<zeno::vec3f>();
                arr[i] = zeno::mix(a, b, f);
            } else {
                float f(drand48());
                auto a = min->get<float>();
                auto b = min->get<float>();
                arr[i] = zeno::mix(a, b, f);
            }
        }
    }, arr);

    set_output("prim", get_input("prim"));
  }
};

ZENDEFNODE(PrimitiveRandomAttr,
    { /* inputs: */ {
    "prim",
    {"NumericObject", "min", "-1"},
    {"NumericObject", "max", "1"},
    }, /* outputs: */ {
    "prim",
    }, /* params: */ {
    {"string", "attrName", "pos"},
    {"enum float float3", "attrType", ""},
    }, /* category: */ {
    "primitive",
    }});

}
