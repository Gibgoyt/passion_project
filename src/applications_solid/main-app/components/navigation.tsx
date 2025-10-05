import { createSignal, Show, For } from 'solid-js'
import type { Component } from 'solid-js'

export type Page = 'dashboard' | 'counter'

interface NavigationItem {
  id: Page
  label: string
  icon: any
}

interface NavigationProps {
  currentPage: Page
  onPageChange: (page: Page) => void
  isDark: boolean
}

const Navigation: Component<NavigationProps> = (props) => {
  const [sidebarCollapsed, setSidebarCollapsed] = createSignal(false)

  const navigationItems: NavigationItem[] = [
    {
      id: 'dashboard',
      label: 'Dashboard',
      icon: (
        <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6"/>
        </svg>
      )
    },
    {
      id: 'counter',
      label: 'Counter',
      icon: (
        <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z"/>
        </svg>
      )
    }
  ]

  return (
    <aside class={`${sidebarCollapsed() ? 'w-16' : 'w-64'} transition-all duration-300 ${props.isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'} border-r flex flex-col h-full fixed left-0 top-0 z-10`}>
      <div class="p-4 border-b border-inherit">
        <div class="flex items-center justify-between">
          <Show when={!sidebarCollapsed()}>
            <h1 class="text-xl font-bold">SolidJS SPA</h1>
          </Show>
          <button
            onClick={() => setSidebarCollapsed(!sidebarCollapsed())}
            class={`p-2 rounded-lg hover:${props.isDark ? 'bg-zinc-700' : 'bg-gray-100'} transition-colors`}
          >
            {sidebarCollapsed() ? '→' : '←'}
          </button>
        </div>
      </div>

      <nav class="flex-1 p-4 overflow-y-auto">
        <ul class="space-y-2">
          <For each={navigationItems}>
            {(item) => (
              <li>
                <button
                  onClick={() => props.onPageChange(item.id)}
                  class={`w-full flex items-center gap-3 px-3 py-2 rounded-lg transition-colors ${
                    props.currentPage === item.id
                      ? props.isDark ? 'bg-blue-600 text-white' : 'bg-blue-100 text-blue-700'
                      : props.isDark ? 'hover:bg-zinc-700 text-gray-300' : 'hover:bg-gray-100 text-gray-700'
                  }`}
                >
                  <span class="text-lg">{item.icon}</span>
                  <Show when={!sidebarCollapsed()}>
                    <span class="font-medium">{item.label}</span>
                  </Show>
                </button>
              </li>
            )}
          </For>
        </ul>
      </nav>

      <div class="p-4 border-t border-inherit">
        <Show when={!sidebarCollapsed()}>
          <a
            href="/apps"
            class={`flex items-center gap-2 px-3 py-2 rounded-lg transition-colors ${props.isDark ? 'hover:bg-zinc-700 text-gray-300' : 'hover:bg-gray-100 text-gray-700'}`}
            onClick={() => window.location.href = '/apps'}
          >
            <span class="text-lg">←</span>
            <span class="font-medium">Back to Apps Hub</span>
          </a>
        </Show>
        <Show when={sidebarCollapsed()}>
          <button
            onClick={() => window.location.href = '/apps'}
            class={`w-full p-2 rounded-lg transition-colors ${props.isDark ? 'hover:bg-zinc-700' : 'hover:bg-gray-100'}`}
            title="Back to Apps Hub"
          >
            ←
          </button>
        </Show>
      </div>
    </aside>
  )
}

export default Navigation