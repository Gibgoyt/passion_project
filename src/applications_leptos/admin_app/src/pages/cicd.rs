use leptos::*;

#[component]
pub fn Cicd() -> impl IntoView {
    view! {
        <div class="p-8 bg-gray-50 dark:bg-zinc-900 min-h-screen">
            <div class="mb-8">
                <h1 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">"CI/CD Pipelines"</h1>
                <p class="text-gray-600 dark:text-gray-400">"Manage your continuous integration and deployment workflows"</p>
            </div>

            <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-sm">
                <div class="p-6 border-b border-gray-200 dark:border-zinc-700 flex items-center justify-between">
                    <h2 class="text-xl font-semibold text-gray-900 dark:text-white">"Recent Deployments"</h2>
                    <button class="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-lg font-semibold transition flex items-center gap-2">
                        <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M14.752 11.168l-3.197-2.132A1 1 0 0010 9.87v4.263a1 1 0 001.555.832l3.197-2.132a1 1 0 000-1.664z"></path>
                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path>
                        </svg>
                        "Deploy"
                    </button>
                </div>

                <div class="p-6 space-y-4">
                    // Deployment 1: Production
                    <div class="border border-gray-200 dark:border-zinc-700 rounded-lg p-6">
                        <div class="flex items-start justify-between mb-4">
                            <div class="flex-1">
                                <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-1">"Production Deploy"</h3>
                                <p class="text-sm text-gray-600 dark:text-gray-400">"v2.1.4 - Feature updates and bug fixes"</p>
                            </div>
                            <span class="px-3 py-1 bg-green-100 dark:bg-green-900/30 text-green-700 dark:text-green-400 text-xs font-semibold rounded-full">"SUCCESS"</span>
                        </div>
                        <div class="flex items-center gap-4 text-sm text-gray-600 dark:text-gray-400">
                            <span class="px-2 py-1 bg-gray-100 dark:bg-zinc-700 rounded font-mono text-xs">"#d1b2c3d"</span>
                            <span class="px-2 py-1 bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-400 rounded text-xs font-medium">"main"</span>
                            <span class="ml-auto text-xs">"2 hours ago"</span>
                        </div>
                    </div>

                    // Deployment 2: Staging
                    <div class="border border-gray-200 dark:border-zinc-700 rounded-lg p-6">
                        <div class="flex items-start justify-between mb-4">
                            <div class="flex-1">
                                <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-1">"Staging Deploy"</h3>
                                <p class="text-sm text-gray-600 dark:text-gray-400">"v2.1.5 - Performance improvements"</p>
                            </div>
                            <span class="px-3 py-1 bg-cyan-100 dark:bg-cyan-900/30 text-cyan-700 dark:text-cyan-400 text-xs font-semibold rounded-full">"RUNNING"</span>
                        </div>
                        <div class="flex items-center gap-4 text-sm text-gray-600 dark:text-gray-400">
                            <span class="px-2 py-1 bg-gray-100 dark:bg-zinc-700 rounded font-mono text-xs">"#e4f5g6h"</span>
                            <span class="px-2 py-1 bg-purple-100 dark:bg-purple-900/30 text-purple-700 dark:text-purple-400 rounded text-xs font-medium">"develop"</span>
                            <span class="ml-auto text-xs">"30 minutes ago"</span>
                        </div>
                    </div>

                    // Deployment 3: Development
                    <div class="border border-gray-200 dark:border-zinc-700 rounded-lg p-6">
                        <div class="flex items-start justify-between mb-4">
                            <div class="flex-1">
                                <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-1">"Development Deploy"</h3>
                                <p class="text-sm text-gray-600 dark:text-gray-400">"v2.2.0 - New features and UI updates"</p>
                            </div>
                            <span class="px-3 py-1 bg-gray-200 dark:bg-gray-700 text-gray-700 dark:text-gray-300 text-xs font-semibold rounded-full">"PENDING"</span>
                        </div>
                        <div class="flex items-center gap-4 text-sm text-gray-600 dark:text-gray-400">
                            <span class="px-2 py-1 bg-gray-100 dark:bg-zinc-700 rounded font-mono text-xs">"#f7j8k91"</span>
                            <span class="px-2 py-1 bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-400 rounded text-xs font-medium">"feature/new-ui"</span>
                            <span class="ml-auto text-xs">"Scheduled"</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    }
}
