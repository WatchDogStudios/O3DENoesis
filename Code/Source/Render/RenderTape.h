#pragma once

#include <AzCore/Debug/Trace.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/containers/span.h>
#include <AzCore/std/containers/vector.h>

namespace NoesisGUI
{
    template<class Draw, class Target, class Texture>
    class RenderTape
    {
    public:
        struct Scope
        {
            Target* m_target = nullptr;
            AZ::u32 m_firstDraw = 0;
            AZ::u32 m_drawCount = 0;
            AZStd::vector<const Texture*> m_sampledRenderTargets;
        };

        void Reset()
        {
            m_scopes.clear();
            m_draws.clear();
            m_open = false;
        }

        void SetTarget(Target* target)
        {
            DropEmptyOpenScope();
            Scope scope;
            scope.m_target = target;
            scope.m_firstDraw = static_cast<AZ::u32>(m_draws.size());
            m_scopes.push_back(AZStd::move(scope));
            m_open = true;
        }

        void AddDraw(Draw&& draw, AZStd::span<const Texture* const> sampledRenderTargets)
        {
            AZ_Assert(m_open, "RenderTape::AddDraw before SetTarget");
            Scope& scope = m_scopes.back();
            m_draws.push_back(AZStd::move(draw));
            ++scope.m_drawCount;
            for (const Texture* texture : sampledRenderTargets)
            {
                if (AZStd::find(scope.m_sampledRenderTargets.begin(), scope.m_sampledRenderTargets.end(), texture) ==
                    scope.m_sampledRenderTargets.end())
                {
                    scope.m_sampledRenderTargets.push_back(texture);
                }
            }
        }

        const AZStd::vector<Scope>& GetScopes()
        {
            DropEmptyOpenScope();
            return m_scopes;
        }

        const AZStd::vector<Draw>& GetDraws() const
        {
            return m_draws;
        }

        AZ::u32 GetOnscreenDrawCount() const
        {
            AZ::u32 count = 0;
            for (const Scope& scope : m_scopes)
            {
                count += scope.m_target ? 0 : scope.m_drawCount;
            }
            return count;
        }

    private:
        void DropEmptyOpenScope()
        {
            if (m_open && m_scopes.back().m_drawCount == 0)
            {
                m_scopes.pop_back();
                m_open = false;
            }
        }

        AZStd::vector<Scope> m_scopes;
        AZStd::vector<Draw> m_draws;
        bool m_open = false;
    };
}
