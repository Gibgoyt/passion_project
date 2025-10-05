import type { Component } from 'solid-js'
import { createSignal, For } from 'solid-js'
import { SummaryCard } from './components/summary-card'

interface DashboardPageProps {
  isDark: boolean
}

const DashboardPage: Component<DashboardPageProps> = (props) => {
  const summaryCards = [
    {
      label: 'Total Projects',
      value: '12',
      icon: (
        <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z" />
        </svg>
      )
    },
    {
      label: 'Files Processed',
      value: '1,842',
      icon: (
        <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z" />
        </svg>
      )
    },
    {
      label: 'Docs Generated',
      value: '1,789',
      icon: (
        <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M4 19.5A2.5 2.5 0 016.5 17H20" />
          <path d="M6.5 2H20v20H6.5A2.5 2.5 0 014 19.5v-15A2.5 2.5 0 016.5 2z" />
        </svg>
      )
    },
    {
      label: 'Time Saved',
      value: '24 hours',
      icon: (
        <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <circle cx="12" cy="12" r="10" />
          <path d="M12 6v6l4 2" />
        </svg>
      )
    }
  ]

  const [recentProjects] = createSignal([
    { name: 'E-commerce Platform', files: 247, lastUpdated: '2 hours ago', status: 'completed' },
    { name: 'React Dashboard', files: 89, lastUpdated: '1 day ago', status: 'in-progress' },
    { name: 'Mobile App Backend', files: 156, lastUpdated: '3 days ago', status: 'pending' }
  ])

  return (
    <div class="p-8">
      <div class="max-w-7xl mx-auto">
        {/* Header */}
        <div class="mb-8">
          <h1 class="text-3xl font-bold mb-2">Dashboard</h1>
          <p class={`${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
            Welcome back! Here's what's happening with your documentation projects.
          </p>
        </div>

        {/* Stats Cards */}
        <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
          <For each={summaryCards}>
            {(card) => (
              <SummaryCard
                label={card.label}
                value={card.value}
                icon={card.icon}
                isDark={props.isDark}
              />
            )}
          </For>
        </div>

        {/* Recent Projects */}
        <div class={`p-6 rounded-lg ${props.isDark ? 'bg-zinc-800' : 'bg-white'} shadow-sm mb-8`}>
          <div class="flex items-center justify-between mb-6">
            <h2 class="text-xl font-semibold">Recent Projects</h2>
            <button class="bg-blue-600 hover:bg-blue-700 text-white font-semibold py-2 px-4 rounded-lg transition-colors">
              New Project
            </button>
          </div>

          <div class="space-y-4">
            {recentProjects().map((project) => (
              <div class={`p-4 rounded-lg border ${props.isDark ? 'border-zinc-700 bg-zinc-900' : 'border-gray-200 bg-gray-50'}`}>
                <div class="flex items-center justify-between">
                  <div class="flex-1">
                    <h3 class="font-semibold mb-1">{project.name}</h3>
                    <p class={`text-sm ${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
                      {project.files} files • Last updated {project.lastUpdated}
                    </p>
                  </div>
                  <div class="flex items-center gap-3">
                    <span class={`px-3 py-1 rounded-full text-xs font-medium ${
                      project.status === 'completed' 
                        ? 'bg-green-100 text-green-800' 
                        : project.status === 'in-progress'
                        ? 'bg-yellow-100 text-yellow-800'
                        : 'bg-gray-100 text-gray-800'
                    }`}>
                      {project.status}
                    </span>
                    <button class={`p-2 rounded-lg hover:${props.isDark ? 'bg-zinc-700' : 'bg-gray-200'} transition-colors`}>
                      →
                    </button>
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Quick Actions */}
        <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          <div class={`p-6 rounded-lg border-2 border-dashed ${props.isDark ? 'border-zinc-700' : 'border-gray-300'} text-center`}>
            <div class="mb-4">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class={`mx-auto ${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
                <path d="M21 15v4a2 2 0 01-2 2H5a2 2 0 01-2-2v-4" />
                <polyline points="17 8 12 3 7 8" />
                <line x1="12" y1="3" x2="12" y2="15" />
              </svg>
            </div>
            <h3 class="font-semibold mb-2">Upload Codebase</h3>
            <p class={`text-sm ${props.isDark ? 'text-gray-400' : 'text-gray-600'} mb-4`}>
              Drag and drop or browse to upload your project files
            </p>
            <button class="bg-blue-600 hover:bg-blue-700 text-white font-semibold py-2 px-4 rounded-lg transition-colors">
              Browse Files
            </button>
          </div>

          <div class={`p-6 rounded-lg border-2 border-dashed ${props.isDark ? 'border-zinc-700' : 'border-gray-300'} text-center`}>
            <div class="mb-4">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class={`mx-auto ${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
                <path d="M10 13a5 5 0 007.54.54l3-3a5 5 0 00-7.07-7.07l-1.72 1.71" />
                <path d="M14 11a5 5 0 00-7.54-.54l-3 3a5 5 0 007.07 7.07l1.71-1.71" />
              </svg>
            </div>
            <h3 class="font-semibold mb-2">Connect Repository</h3>
            <p class={`text-sm ${props.isDark ? 'text-gray-400' : 'text-gray-600'} mb-4`}>
              Connect your GitHub, GitLab, or Bitbucket repository
            </p>
            <button class="bg-green-600 hover:bg-green-700 text-white font-semibold py-2 px-4 rounded-lg transition-colors">
              Connect Git
            </button>
          </div>

          <div class={`p-6 rounded-lg border-2 border-dashed ${props.isDark ? 'border-zinc-700' : 'border-gray-300'} text-center`}>
            <div class="mb-4">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class={`mx-auto ${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
                <rect x="3" y="11" width="18" height="10" rx="2" ry="2" />
                <circle cx="12" cy="5" r="2" />
                <path d="M12 7v4" />
                <line x1="8" y1="16" x2="8" y2="16" />
                <line x1="16" y1="16" x2="16" y2="16" />
              </svg>
            </div>
            <h3 class="font-semibold mb-2">AI Assistant</h3>
            <p class={`text-sm ${props.isDark ? 'text-gray-400' : 'text-gray-600'} mb-4`}>
              Get help with documentation generation and optimization
            </p>
            <button class="bg-purple-600 hover:bg-purple-700 text-white font-semibold py-2 px-4 rounded-lg transition-colors">
              Ask AI
            </button>
          </div>
        </div>
      </div>
    </div>
  )
}

export default DashboardPage