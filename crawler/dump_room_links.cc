//
// TODO(syoyo): Print extensions and extras for each glTF object.
//
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <cstdio>
#include <fstream>
#include <iostream>

static std::string GetFilePathExtension(const std::string &FileName) {
  if (FileName.find_last_of(".") != std::string::npos)
    return FileName.substr(FileName.find_last_of(".") + 1);
  return "";
}

static std::string PrintMode(int mode) {
  if (mode == TINYGLTF_MODE_POINTS) {
    return "POINTS";
  } else if (mode == TINYGLTF_MODE_LINE) {
    return "LINE";
  } else if (mode == TINYGLTF_MODE_LINE_LOOP) {
    return "LINE_LOOP";
  } else if (mode == TINYGLTF_MODE_TRIANGLES) {
    return "TRIANGLES";
  } else if (mode == TINYGLTF_MODE_TRIANGLE_FAN) {
    return "TRIANGLE_FAN";
  } else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
    return "TRIANGLE_STRIP";
  }
  return "**UNKNOWN**";
}

static std::string PrintTarget(int target) {
  if (target == 34962) {
    return "GL_ARRAY_BUFFER";
  } else if (target == 34963) {
    return "GL_ELEMENT_ARRAY_BUFFER";
  } else {
    return "**UNKNOWN**";
  }
}

static std::string PrintType(int ty) {
  if (ty == TINYGLTF_TYPE_SCALAR) {
    return "SCALAR";
  } else if (ty == TINYGLTF_TYPE_VECTOR) {
    return "VECTOR";
  } else if (ty == TINYGLTF_TYPE_VEC2) {
    return "VEC2";
  } else if (ty == TINYGLTF_TYPE_VEC3) {
    return "VEC3";
  } else if (ty == TINYGLTF_TYPE_VEC4) {
    return "VEC4";
  } else if (ty == TINYGLTF_TYPE_MATRIX) {
    return "MATRIX";
  } else if (ty == TINYGLTF_TYPE_MAT2) {
    return "MAT2";
  } else if (ty == TINYGLTF_TYPE_MAT3) {
    return "MAT3";
  } else if (ty == TINYGLTF_TYPE_MAT4) {
    return "MAT4";
  }
  return "**UNKNOWN**";
}

static std::string PrintComponentType(int ty) {
  if (ty == TINYGLTF_COMPONENT_TYPE_BYTE) {
    return "BYTE";
  } else if (ty == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
    return "UNSIGNED_BYTE";
  } else if (ty == TINYGLTF_COMPONENT_TYPE_SHORT) {
    return "SHORT";
  } else if (ty == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
    return "UNSIGNED_SHORT";
  } else if (ty == TINYGLTF_COMPONENT_TYPE_INT) {
    return "INT";
  } else if (ty == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
    return "UNSIGNED_INT";
  } else if (ty == TINYGLTF_COMPONENT_TYPE_FLOAT) {
    return "FLOAT";
  } else if (ty == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
    return "DOUBLE";
  }

  return "**UNKNOWN**";
}

#if 0
static std::string PrintParameterType(int ty) {
  if (ty == TINYGLTF_PARAMETER_TYPE_BYTE) {
    return "BYTE";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE) {
    return "UNSIGNED_BYTE";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_SHORT) {
    return "SHORT";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) {
    return "UNSIGNED_SHORT";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_INT) {
    return "INT";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT) {
    return "UNSIGNED_INT";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT) {
    return "FLOAT";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC2) {
    return "FLOAT_VEC2";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3) {
    return "FLOAT_VEC3";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC4) {
    return "FLOAT_VEC4";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_INT_VEC2) {
    return "INT_VEC2";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_INT_VEC3) {
    return "INT_VEC3";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_INT_VEC4) {
    return "INT_VEC4";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_BOOL) {
    return "BOOL";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_BOOL_VEC2) {
    return "BOOL_VEC2";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_BOOL_VEC3) {
    return "BOOL_VEC3";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_BOOL_VEC4) {
    return "BOOL_VEC4";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_MAT2) {
    return "FLOAT_MAT2";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_MAT3) {
    return "FLOAT_MAT3";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_MAT4) {
    return "FLOAT_MAT4";
  } else if (ty == TINYGLTF_PARAMETER_TYPE_SAMPLER_2D) {
    return "SAMPLER_2D";
  }

  return "**UNKNOWN**";
}
#endif

static std::string PrintWrapMode(int mode) {
  if (mode == TINYGLTF_TEXTURE_WRAP_REPEAT) {
    return "REPEAT";
  } else if (mode == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE) {
    return "CLAMP_TO_EDGE";
  } else if (mode == TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT) {
    return "MIRRORED_REPEAT";
  }

  return "**UNKNOWN**";
}

static std::string PrintFilterMode(int mode) {
  if (mode == TINYGLTF_TEXTURE_FILTER_NEAREST) {
    return "NEAREST";
  } else if (mode == TINYGLTF_TEXTURE_FILTER_LINEAR) {
    return "LINEAR";
  } else if (mode == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST) {
    return "NEAREST_MIPMAP_NEAREST";
  } else if (mode == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR) {
    return "NEAREST_MIPMAP_LINEAR";
  } else if (mode == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST) {
    return "LINEAR_MIPMAP_NEAREST";
  } else if (mode == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR) {
    return "LINEAR_MIPMAP_LINEAR";
  }
  return "**UNKNOWN**";
}

static std::string PrintIntArray(const std::vector<int> &arr) {
  if (arr.size() == 0) {
    return "";
  }

  std::stringstream ss;
  ss << "[ ";
  for (size_t i = 0; i < arr.size(); i++) {
    ss << arr[i];
    if (i != arr.size() - 1) {
      ss << ", ";
    }
  }
  ss << " ]";

  return ss.str();
}

static std::string PrintStringArray(const std::vector<std::string> &arr) {
  if (arr.size() == 0) {
    return "";
  }

  std::stringstream ss;
  ss << "[ ";
  for (size_t i = 0; i < arr.size(); i++) {
    ss << arr[i];
    if (i != arr.size() - 1) {
      ss << ", ";
    }
  }
  ss << " ]";

  return ss.str();
}

static std::string PrintFloatArray(const std::vector<double> &arr) {
  if (arr.size() == 0) {
    return "";
  }

  std::stringstream ss;
  ss << "[ ";
  for (size_t i = 0; i < arr.size(); i++) {
    ss << arr[i];
    if (i != arr.size() - 1) {
      ss << ", ";
    }
  }
  ss << " ]";

  return ss.str();
}

static std::string Indent(const int indent) {
  std::string s;
  for (int i = 0; i < indent; i++) {
    s += "  ";
  }

  return s;
}

static std::string PrintParameterValue(const tinygltf::Parameter &param) {
  if (!param.number_array.empty()) {
    return PrintFloatArray(param.number_array);
  } else {
    return param.string_value;
  }
}

#if 0
static std::string PrintParameterMap(const tinygltf::ParameterMap &pmap) {
  std::stringstream ss;

  ss << pmap.size() << std::endl;
  for (auto &kv : pmap) {
    ss << kv.first << " : " << PrintParameterValue(kv.second) << std::endl;
  }

  return ss.str();
}
#endif

static std::string PrintValue(const std::string &name,
                              const tinygltf::Value &value, const int indent,
                              const bool tag = true) {
  std::stringstream ss;

  if (value.IsObject()) {
    const tinygltf::Value::Object &o = value.Get<tinygltf::Value::Object>();
    tinygltf::Value::Object::const_iterator it(o.begin());
    tinygltf::Value::Object::const_iterator itEnd(o.end());
    for (; it != itEnd; it++) {
      ss << PrintValue(it->first, it->second, indent + 1) << std::endl;
    }
  } else if (value.IsString()) {
    if (tag) {
      ss << Indent(indent) << name << " : " << value.Get<std::string>();
    } else {
      ss << Indent(indent) << value.Get<std::string>() << " ";
    }
  } else if (value.IsBool()) {
    if (tag) {
      ss << Indent(indent) << name << " : " << value.Get<bool>();
    } else {
      ss << Indent(indent) << value.Get<bool>() << " ";
    }
  } else if (value.IsNumber()) {
    if (tag) {
      ss << Indent(indent) << name << " : " << value.Get<double>();
    } else {
      ss << Indent(indent) << value.Get<double>() << " ";
    }
  } else if (value.IsInt()) {
    if (tag) {
      ss << Indent(indent) << name << " : " << value.Get<int>();
    } else {
      ss << Indent(indent) << value.Get<int>() << " ";
    }
  } else if (value.IsArray()) {
    // TODO(syoyo): Better pretty printing of array item
    ss << Indent(indent) << name << " [ \n";
    for (size_t i = 0; i < value.Size(); i++) {
      ss << PrintValue("", value.Get(int(i)), indent + 1, /* tag */ false);
      if (i != (value.ArrayLen() - 1)) {
        ss << ", \n";
      }
    }
    ss << "\n" << Indent(indent) << "] ";
  }

  // @todo { binary }

  return ss.str();
}

static void DumpNode(const tinygltf::Node &node, int indent) {
  std::cout << Indent(indent) << "name        : " << node.name << std::endl;
  std::cout << Indent(indent) << "camera      : " << node.camera << std::endl;
  std::cout << Indent(indent) << "mesh        : " << node.mesh << std::endl;
  if (!node.rotation.empty()) {
    std::cout << Indent(indent)
              << "rotation    : " << PrintFloatArray(node.rotation)
              << std::endl;
  }
  if (!node.scale.empty()) {
    std::cout << Indent(indent)
              << "scale       : " << PrintFloatArray(node.scale) << std::endl;
  }
  if (!node.translation.empty()) {
    std::cout << Indent(indent)
              << "translation : " << PrintFloatArray(node.translation)
              << std::endl;
  }

  if (!node.matrix.empty()) {
    std::cout << Indent(indent)
              << "matrix      : " << PrintFloatArray(node.matrix) << std::endl;
  }

  std::cout << Indent(indent)
            << "children    : " << PrintIntArray(node.children) << std::endl;

    //std::cout << "        lal: " << node.extras.Get(i).Keys << std::endl;
  for (auto &b : node.extensions) {
    std::cout << "        lal: " << b.first << std::endl;
    std::cout << "        ehj: " << PrintStringArray(b.second.Keys()) << std::endl;

    auto ext = b.second;
    for (auto k : b.second.Keys()) {
      std::cout << "--------------------" << std::endl;
      std::cout << "              " << k << std::endl;
      std::cout << "          type: " << (int)ext.IsObject() << std::endl;
      std::cout << "      inner keys: " << PrintStringArray(ext.Keys()) << std::endl;
      json elementJson;
      const tinygltf::Value &val = ext.Get(k);

      if (val.IsObject()) {
        auto objMap = val.Get<tinygltf::Value::Object>();
        for (auto &it : objMap) {
          std::cout << "          " << it.first << ": " << it.second.Get<std::string>() << std::endl;
        }
      } else {
        std::cout << "r not an object" << std::endl;
      }
      auto jj = ValueToJson(val, &elementJson);
      //std::cout << "         eh: " << jj << std::endl;
      std::cout << "         ehfl: " << val.Get<std::string>() << std::endl;
      //std::cout << "         ehfl2: " << tinygltf::JsonToString(jj) << std::endl;
      std::cout << "--------------------" << std::endl;

      // print keys from json
      //std::cout << "         eh: " << PrintStringArray(elementJson.keys()) << std::endl;

    }
  }
  //auto &m = node.extras.
  //std::cout << Indent(indent) 
            //<< "json: " << node.extras << std::endl;
}

static void DumpStringIntMap(const std::map<std::string, int> &m, int indent) {
  std::map<std::string, int>::const_iterator it(m.begin());
  std::map<std::string, int>::const_iterator itEnd(m.end());
  for (; it != itEnd; it++) {
    std::cout << Indent(indent) << it->first << ": " << it->second << std::endl;
  }
}

static void DumpExtensions(const tinygltf::ExtensionMap &extension,
                           const int indent) {
  // TODO(syoyo): pritty print Value
  for (auto &e : extension) {
    std::cout << Indent(indent) << e.first << std::endl;
    std::cout << PrintValue("extensions", e.second, indent + 1) << std::endl;
  }
}

static void DumpPrimitive(const tinygltf::Primitive &primitive, int indent) {
  std::cout << Indent(indent) << "material : " << primitive.material
            << std::endl;
  std::cout << Indent(indent) << "indices : " << primitive.indices << std::endl;
  std::cout << Indent(indent) << "mode     : " << PrintMode(primitive.mode)
            << "(" << primitive.mode << ")" << std::endl;
  std::cout << Indent(indent)
            << "attributes(items=" << primitive.attributes.size() << ")"
            << std::endl;
  DumpStringIntMap(primitive.attributes, indent + 1);

  DumpExtensions(primitive.extensions, indent);
  std::cout << Indent(indent) << "extras :" << std::endl
            << PrintValue("extras", primitive.extras, indent + 1) << std::endl;

  if (!primitive.extensions_json_string.empty()) {
    std::cout << Indent(indent + 1) << "extensions(JSON string) = "
              << primitive.extensions_json_string << "\n";
  }

  if (!primitive.extras_json_string.empty()) {
    std::cout << Indent(indent + 1)
              << "extras(JSON string) = " << primitive.extras_json_string
              << "\n";
  }
}


static void DumpTextureInfo(const tinygltf::TextureInfo &texinfo,
                            const int indent) {
  std::cout << Indent(indent) << "index     : " << texinfo.index << "\n";
  std::cout << Indent(indent) << "texCoord  : TEXCOORD_" << texinfo.texCoord
            << "\n";
  DumpExtensions(texinfo.extensions, indent + 1);
  std::cout << PrintValue("extras", texinfo.extras, indent + 1) << "\n";

  if (!texinfo.extensions_json_string.empty()) {
    std::cout << Indent(indent)
              << "extensions(JSON string) = " << texinfo.extensions_json_string
              << "\n";
  }

  if (!texinfo.extras_json_string.empty()) {
    std::cout << Indent(indent)
              << "extras(JSON string) = " << texinfo.extras_json_string << "\n";
  }
}

static void DumpNormalTextureInfo(const tinygltf::NormalTextureInfo &texinfo,
                                  const int indent) {
  std::cout << Indent(indent) << "index     : " << texinfo.index << "\n";
  std::cout << Indent(indent) << "texCoord  : TEXCOORD_" << texinfo.texCoord
            << "\n";
  std::cout << Indent(indent) << "scale     : " << texinfo.scale << "\n";
  DumpExtensions(texinfo.extensions, indent + 1);
  std::cout << PrintValue("extras", texinfo.extras, indent + 1) << "\n";
}

static void DumpOcclusionTextureInfo(
    const tinygltf::OcclusionTextureInfo &texinfo, const int indent) {
  std::cout << Indent(indent) << "index     : " << texinfo.index << "\n";
  std::cout << Indent(indent) << "texCoord  : TEXCOORD_" << texinfo.texCoord
            << "\n";
  std::cout << Indent(indent) << "strength  : " << texinfo.strength << "\n";
  DumpExtensions(texinfo.extensions, indent + 1);
  std::cout << PrintValue("extras", texinfo.extras, indent + 1) << "\n";
}

static void DumpPbrMetallicRoughness(const tinygltf::PbrMetallicRoughness &pbr,
                                     const int indent) {
  std::cout << Indent(indent)
            << "baseColorFactor   : " << PrintFloatArray(pbr.baseColorFactor)
            << "\n";
  std::cout << Indent(indent) << "baseColorTexture  :\n";
  DumpTextureInfo(pbr.baseColorTexture, indent + 1);

  std::cout << Indent(indent) << "metallicFactor    : " << pbr.metallicFactor
            << "\n";
  std::cout << Indent(indent) << "roughnessFactor   : " << pbr.roughnessFactor
            << "\n";

  std::cout << Indent(indent) << "metallicRoughnessTexture  :\n";
  DumpTextureInfo(pbr.metallicRoughnessTexture, indent + 1);
  DumpExtensions(pbr.extensions, indent + 1);
  std::cout << PrintValue("extras", pbr.extras, indent + 1) << "\n";
}

static void Dump(const tinygltf::Model &model) {
    //std::cout << "nodes(items=" << model.nodes.size() << ")" << std::endl;
    for (size_t i = 0; i < model.nodes.size(); i++) {
      const tinygltf::Node &node = model.nodes[i];
      //std::cout << Indent(1) << "name         : " << node.name << std::endl;

      for (auto &b : node.extensions) {
        //std::cout << "        lal: " << b.first << std::endl;
        //std::cout << "        ehj: " << PrintStringArray(b.second.Keys()) << std::endl;

        auto ext = b.second;
        for (auto k : b.second.Keys()) {
          if (k != "link") continue;
          const tinygltf::Value &val = ext.Get(k);

          if (val.IsObject()) {
            auto objMap = val.Get<tinygltf::Value::Object>();
            if (objMap.find("href") != objMap.end()) {
              const tinygltf::Value &nodeVal = objMap["href"];
              if (nodeVal.IsString()) {
                std::cout << nodeVal.Get<std::string>() << std::endl;
              } else {
                std::cout << "href is not a string" << std::endl;
                exit(10);
              }
            }
          } else {
            std::cout << "r not an object" << std::endl;
            exit(9);
          }
        }
      }

      //DumpNode(node, 2);
    }


}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Needs input.gltf\n");
    exit(1);
  }

  // Store original JSON string for `extras` and `extensions`
  bool store_original_json_for_extras_and_extensions = false;
  if (argc > 2) {
    store_original_json_for_extras_and_extensions = true;
  }

  tinygltf::Model model;
  tinygltf::TinyGLTF gltf_ctx;
  std::string err;
  std::string warn;
  std::string input_filename(argv[1]);
  std::string ext = GetFilePathExtension(input_filename);

  gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(
      store_original_json_for_extras_and_extensions);

  bool ret = false;
  // assume binary glTF.
  ret = gltf_ctx.LoadBinaryFromFile(&model, &err, &warn,
                                    input_filename.c_str());

  if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
  }

  if (!err.empty()) {
    printf("Err: %s\n", err.c_str());
  }

  if (!ret) {
    printf("Failed to parse glTF\n");
    return -1;
  }

  Dump(model);

  return 0;
}
