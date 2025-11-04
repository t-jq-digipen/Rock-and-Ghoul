//#pragma once
//#include <mono/jit/jit.h>
//#include <mono/metadata/assembly.h>
//#include <mono/metadata/debug-helpers.h>
//#include <string>
//#include <stdexcept>
//
//class MonoManager
//{
//public:
//    static MonoManager& Get()
//    {
//        static MonoManager instance;
//        return instance;
//    }
//
//    void Initialize(const std::string& assemblyPath);
//    void InvokeMethod(const std::string& className, const std::string& methodName, float dt);
//    void Shutdown();
//    MonoDomain* GetDomain() { return domain; }
//    MonoImage* GetImage() { return image; }
//private:
//    MonoManager() = default;
//    ~MonoManager() = default;
//
//    MonoDomain* domain = nullptr;
//    MonoAssembly* assembly = nullptr;
//    MonoImage* image = nullptr;
//};
