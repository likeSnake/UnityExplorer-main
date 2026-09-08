#include "er2/unity2/metadata/il2cpp_layout_profile.hpp"

#include <cstdio>

namespace {

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }

  std::printf("[il2cpp_profile] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  bool ok = true;

  const er2::Il2CppLayoutProfile *profile =
      er2::FindIl2CppLayoutProfileForUnity2019_4("2019.4.41");
  ok &= Expect(profile != nullptr,
               "Unity 2019.4.41 should resolve to a 2019.4 LTS profile");

  if (profile) {
    ok &= Expect(profile->metadataVersionMajor == 24,
                 "Unity 2019.4 LTS candidate should use metadata version 24");
    ok &= Expect(profile->metadataVersionMinor == 5,
                 "Unity 2019.4.41 candidate should prefer 24.5 layout");
    ok &= Expect(profile->klass.name == 0x10,
                 "Il2CppClass.name offset should be 0x10");
    ok &= Expect(profile->klass.namespaze == 0x18,
                 "Il2CppClass.namespaze offset should be 0x18");
    ok &= Expect(profile->klass.fields == 0x80,
                 "Il2CppClass.fields offset should be 0x80");
    ok &= Expect(profile->klass.methods == 0x98,
                 "Il2CppClass.methods offset should be 0x98");
    ok &= Expect(profile->klass.staticFields == 0xB8,
                 "Il2CppClass.static_fields offset should be 0xB8");
    ok &= Expect(profile->klass.methodCount == 0x118,
                 "Il2CppClass.method_count offset should be 0x118");
    ok &= Expect(profile->klass.fieldCount == 0x11C,
                 "Il2CppClass.field_count offset should be 0x11C");
    ok &= Expect(profile->fieldInfo.name == 0x00,
                 "FieldInfo.name offset should be 0x00");
    ok &= Expect(profile->fieldInfo.type == 0x08,
                 "FieldInfo.type offset should be 0x08");
    ok &= Expect(profile->fieldInfo.parent == 0x10,
                 "FieldInfo.parent offset should be 0x10");
    ok &= Expect(profile->fieldInfo.offset == 0x18,
                 "FieldInfo.offset offset should be 0x18");
    ok &= Expect(profile->fieldInfo.size == 0x20,
                 "FieldInfo size should be 0x20 on x64");
    ok &= Expect(profile->methodInfo.name == 0x10,
                 "MethodInfo.name offset should be 0x10");
    ok &= Expect(profile->methodInfo.klass == 0x18,
                 "MethodInfo.klass offset should be 0x18");
    ok &= Expect(profile->methodInfo.returnType == 0x20,
                 "MethodInfo.return_type offset should be 0x20");
    ok &= Expect(profile->methodInfo.parameters == 0x28,
                 "MethodInfo.parameters offset should be 0x28");
    ok &= Expect(profile->methodInfo.token == 0x48,
                 "MethodInfo.token offset should be 0x48");
    ok &= Expect(profile->methodInfo.flags == 0x4C,
                 "MethodInfo.flags offset should be 0x4C");
    ok &= Expect(profile->methodInfo.slot == 0x50,
                 "MethodInfo.slot offset should be 0x50");
    ok &= Expect(profile->methodInfo.parameterCount == 0x52,
                 "MethodInfo.parameters_count offset should be 0x52");
    ok &= Expect(profile->metadataRegistration.types == 0x38,
                 "MetadataRegistration.types offset should be 0x38");
    ok &= Expect(profile->metadataRegistration.fieldOffsets == 0x58,
                 "MetadataRegistration.fieldOffsets offset should be 0x58");
  }

  ok &= Expect(er2::FindIl2CppLayoutProfileForUnity2019_4("2020.1.1") ==
                   nullptr,
               "Unity 2020 should not resolve through the 2019.4-only helper");

  if (!ok) {
    return 1;
  }

  std::printf("[il2cpp_profile] PASS\n");
  return 0;
}
