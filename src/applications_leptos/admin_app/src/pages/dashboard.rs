use leptos::*;

#[component]
pub fn Dashboard() -> impl IntoView {
    view! {
        <div class="p-8 bg-gray-50 dark:bg-zinc-900 min-h-screen">
            // Page header
            <div class="mb-8">
                <h1 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">"Dashboard"</h1>
                <p class="text-gray-600 dark:text-gray-400">"Welcome back! Here's what's happening with your system."</p>
            </div>

            // Stats cards
            <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
                // Total Users card
                <div class="bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm">
                    <div class="flex items-center justify-between mb-4">
                        <div class="p-3 bg-blue-100 dark:bg-blue-900/30 rounded-lg">
                            <svg class="w-6 h-6 text-blue-600 dark:text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0zm6 3a2 2 0 11-4 0 2 2 0 014 0zM7 10a2 2 0 11-4 0 2 2 0 014 0z"></path>
                            </svg>
                        </div>
                    </div>
                    <h3 class="text-sm font-medium text-gray-600 dark:text-gray-400 mb-1">"Total Users"</h3>
                    <div class="flex items-end justify-between">
                        <p class="text-3xl font-bold text-gray-900 dark:text-white">"1,250"</p>
                        <span class="text-sm font-medium text-green-600 dark:text-green-400">"↑ +12%"</span>
                    </div>
                </div>

                // Database Tables card
                <div class="bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm">
                    <div class="flex items-center justify-between mb-4">
                        <div class="p-3 bg-blue-100 dark:bg-blue-900/30 rounded-lg">
                            <svg class="w-6 h-6 text-blue-600 dark:text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 7v10c0 2.21 3.582 4 8 4s8-1.79 8-4V7M4 7c0 2.21 3.582 4 8 4s8-1.79 8-4M4 7c0-2.21 3.582-4 8-4s8 1.79 8 4m0 5c0 2.21-3.582 4-8 4s-8-1.79-8-4"></path>
                            </svg>
                        </div>
                    </div>
                    <h3 class="text-sm font-medium text-gray-600 dark:text-gray-400 mb-1">"Database Tables"</h3>
                    <div class="flex items-end justify-between">
                        <p class="text-3xl font-bold text-gray-900 dark:text-white">"45"</p>
                        <span class="text-sm font-medium text-green-600 dark:text-green-400">"↑ +2"</span>
                    </div>
                </div>

                // System Uptime card
                <div class="bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm">
                    <div class="flex items-center justify-between mb-4">
                        <div class="p-3 bg-blue-100 dark:bg-blue-900/30 rounded-lg">
                            <svg class="w-6 h-6 text-blue-600 dark:text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 7h8m0 0v8m0-8l-8 8-4-4-6 6"></path>
                            </svg>
                        </div>
                    </div>
                    <h3 class="text-sm font-medium text-gray-600 dark:text-gray-400 mb-1">"System Uptime"</h3>
                    <div class="flex items-end justify-between">
                        <p class="text-3xl font-bold text-gray-900 dark:text-white">"99.8%"</p>
                        <span class="text-sm font-medium text-green-600 dark:text-green-400">"↑ +0.2%"</span>
                    </div>
                </div>

                // Active Services card
                <div class="bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm">
                    <div class="flex items-center justify-between mb-4">
                        <div class="p-3 bg-blue-100 dark:bg-blue-900/30 rounded-lg">
                            <svg class="w-6 h-6 text-blue-600 dark:text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 12h14M5 12a2 2 0 01-2-2V6a2 2 0 012-2h14a2 2 0 012 2v4a2 2 0 01-2 2M5 12a2 2 0 00-2 2v4a2 2 0 002 2h14a2 2 0 002-2v-4a2 2 0 00-2-2m-2-4h.01M17 16h.01"></path>
                            </svg>
                        </div>
                    </div>
                    <h3 class="text-sm font-medium text-gray-600 dark:text-gray-400 mb-1">"Active Services"</h3>
                    <div class="flex items-end justify-between">
                        <p class="text-3xl font-bold text-gray-900 dark:text-white">"12"</p>
                        <span class="text-sm font-medium text-green-600 dark:text-green-400">"↑ All healthy"</span>
                    </div>
                </div>
            </div>

            // Two-column layout: System Performance + Recent Activity
            <div class="grid grid-cols-1 lg:grid-cols-3 gap-6">
                // System Performance (2/3 width)
                <div class="lg:col-span-2 bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm">
                    <div class="flex items-center justify-between mb-6">
                        <h2 class="text-xl font-semibold text-gray-900 dark:text-white">"System Performance"</h2>
                        <div class="flex gap-2">
                            <button class="px-3 py-1 text-sm font-medium text-gray-600 dark:text-gray-400 hover:bg-gray-100 dark:hover:bg-zinc-700 rounded">"7D"</button>
                            <button class="px-3 py-1 text-sm font-medium bg-blue-600 text-white rounded">"30D"</button>
                        </div>
                    </div>
                    <div class="flex items-center justify-center h-64 text-gray-400 dark:text-gray-500">
                        <div class="text-center">
                            <svg class="w-16 h-16 mx-auto mb-2 text-gray-300 dark:text-gray-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z"></path>
                            </svg>
                            <p class="text-sm">"Performance chart will be rendered here"</p>
                        </div>
                    </div>
                </div>

                // Recent Activity (1/3 width)
                <div class="bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm">
                    <div class="flex items-center justify-between mb-6">
                        <h2 class="text-xl font-semibold text-gray-900 dark:text-white">"Recent Activity"</h2>
                        <button class="px-3 py-1 text-sm font-medium text-blue-600 dark:text-blue-400 hover:bg-blue-50 dark:hover:bg-blue-900/20 rounded">"View All"</button>
                    </div>

                    <div class="space-y-4">
                        // Activity item 1
                        <div class="flex gap-3">
                            <div class="flex-shrink-0 w-8 h-8 bg-green-100 dark:bg-green-900/30 rounded-full flex items-center justify-center">
                                <svg class="w-4 h-4 text-green-600 dark:text-green-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7"></path>
                                </svg>
                            </div>
                            <div class="flex-1 min-w-0">
                                <p class="text-sm font-medium text-gray-900 dark:text-white">"Database backup completed successfully"</p>
                                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">"2 minutes ago"</p>
                            </div>
                        </div>

                        // Activity item 2
                        <div class="flex gap-3">
                            <div class="flex-shrink-0 w-8 h-8 bg-blue-100 dark:bg-blue-900/30 rounded-full flex items-center justify-center">
                                <svg class="w-4 h-4 text-blue-600 dark:text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M16 7a4 4 0 11-8 0 4 4 0 018 0zM12 14a7 7 0 00-7 7h14a7 7 0 00-7-7z"></path>
                                </svg>
                            </div>
                            <div class="flex-1 min-w-0">
                                <p class="text-sm font-medium text-gray-900 dark:text-white">"New user registered: john.doe@example.com"</p>
                                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">"15 minutes ago"</p>
                            </div>
                        </div>

                        // Activity item 3
                        <div class="flex gap-3">
                            <div class="flex-shrink-0 w-8 h-8 bg-orange-100 dark:bg-orange-900/30 rounded-full flex items-center justify-center">
                                <svg class="w-4 h-4 text-orange-600 dark:text-orange-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z"></path>
                                </svg>
                            </div>
                            <div class="flex-1 min-w-0">
                                <p class="text-sm font-medium text-gray-900 dark:text-white">"High memory usage detected on server-03"</p>
                                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">"1 hour ago"</p>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    }
}
