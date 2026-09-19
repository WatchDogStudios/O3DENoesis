#include <Runtime/NoesisRuntime.h>

#include <AzCore/Memory/ChildAllocatorSchema.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Settings/SettingsRegistry.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzCore/std/string/string.h>

#include <NsCore/Error.h>
#include <NsCore/Log.h>
#include <NsCore/Memory.h>
#include <NsGui/IntegrationAPI.h>

namespace NoesisGUI
{
    AZ_CHILD_ALLOCATOR_WITH_NAME(NoesisAllocator, "NoesisAllocator", "{5B0B1D6E-2E6C-4F0A-9D6B-6C7C8E0C4A11}", AZ::SystemAllocator);

    namespace
    {
        constexpr size_t NoesisAlignment = alignof(std::max_align_t);

        AZStd::mutex s_mutex;
        AZ::u32 s_refCount = 0;
        AZ::s64 s_logVerbosity = 2;

        void* Alloc(void*, size_t size)
        {
            return AZ::AllocatorInstance<NoesisAllocator>::Get().allocate(size, NoesisAlignment).GetAddress();
        }

        void* Realloc(void*, void* ptr, size_t size)
        {
            return AZ::AllocatorInstance<NoesisAllocator>::Get().reallocate(ptr, size, NoesisAlignment).GetAddress();
        }

        void Dealloc(void*, void* ptr)
        {
            AZ::AllocatorInstance<NoesisAllocator>::Get().deallocate(ptr, 0, NoesisAlignment);
        }

        size_t AllocSize(void*, void* ptr)
        {
            return AZ::AllocatorInstance<NoesisAllocator>::Get().get_allocated_size(ptr, NoesisAlignment);
        }

        void OnLog(const char* file, uint32_t line, uint32_t level, const char* channel, const char* message)
        {
            // Noesis levels: 0 trace, 1 debug, 2 info, 3 warning, 4 error. Verbosity counts the other way.
            if (level >= 4)
            {
                AZ_Error("NoesisGUI", false, "[%s] %s (%s:%u)", channel, message, file, line);
            }
            else if (level == 3)
            {
                AZ_Warning("NoesisGUI", s_logVerbosity < 1, "[%s] %s", channel, message);
            }
            else if (static_cast<AZ::s64>(4 - level) <= s_logVerbosity)
            {
                AZ_Trace("NoesisGUI", "[%s] %s\n", channel, message);
            }
        }

        void OnError(const char* file, uint32_t line, const char* message, bool fatal)
        {
            AZ_Error("NoesisGUI", false, "%s (%s:%u)", message, file, line);
            AZ_Assert(!fatal, "NoesisGUI fatal error: %s", message);
        }
    }

    namespace NoesisRuntime
    {
        void Acquire()
        {
            AZStd::scoped_lock lock(s_mutex);
            if (s_refCount++ > 0)
            {
                return;
            }

            AZStd::string licenseName;
            AZStd::string licenseKey;
            if (auto* registry = AZ::SettingsRegistry::Get())
            {
                registry->Get(licenseName, "/O3DE/NoesisGUI/LicenseName");
                registry->Get(licenseKey, "/O3DE/NoesisGUI/LicenseKey");
                registry->Get(s_logVerbosity, "/O3DE/NoesisGUI/LogVerbosity");
            }

            Noesis::MemoryCallbacks memory;
            memory.user = nullptr;
            memory.alloc = &Alloc;
            memory.realloc = &Realloc;
            memory.dealloc = &Dealloc;
            memory.allocSize = &AllocSize;
            Noesis::GUI::SetMemoryCallbacks(memory);
            Noesis::GUI::SetLogHandler(&OnLog);
            Noesis::GUI::SetErrorHandler(&OnError);
            Noesis::GUI::SetLicense(licenseName.c_str(), licenseKey.c_str());
            Noesis::GUI::Init();
        }

        void Release()
        {
            AZStd::scoped_lock lock(s_mutex);
            AZ_Assert(s_refCount > 0, "NoesisRuntime::Release without Acquire");
            if (--s_refCount == 0)
            {
                Noesis::GUI::Shutdown();
            }
        }

        AZ::u32 GetRefCount()
        {
            AZStd::scoped_lock lock(s_mutex);
            return s_refCount;
        }
    }
}
