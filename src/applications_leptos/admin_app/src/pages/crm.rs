use leptos::*;

#[component]
pub fn Crm() -> impl IntoView {
    view! {
        <div class="p-8 bg-gray-50 dark:bg-zinc-900 min-h-screen">
            <div class="mb-8">
                <h1 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">"CRM Dashboard"</h1>
                <p class="text-gray-600 dark:text-gray-400">"Manage your customer relationships and sales pipeline"</p>
            </div>

            <div class="grid grid-cols-1 lg:grid-cols-2 gap-6">
                // Recent Contacts
                <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-sm p-6">
                    <div class="flex items-center justify-between mb-6">
                        <h2 class="text-xl font-semibold text-gray-900 dark:text-white">"Recent Contacts"</h2>
                        <button class="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-lg font-semibold transition flex items-center gap-2">
                            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4"></path>
                            </svg>
                            "Add Contact"
                        </button>
                    </div>

                    <div class="space-y-4">
                        // Contact 1
                        <div class="flex items-center justify-between p-4 bg-gray-50 dark:bg-zinc-700/50 rounded-lg">
                            <div class="flex items-center gap-3">
                                <div class="w-10 h-10 rounded-full bg-blue-100 dark:bg-blue-900/30 flex items-center justify-center">
                                    <svg class="w-5 h-5 text-blue-600 dark:text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M16 7a4 4 0 11-8 0 4 4 0 018 0zM12 14a7 7 0 00-7 7h14a7 7 0 00-7-7z"></path>
                                    </svg>
                                </div>
                                <div>
                                    <p class="text-sm font-semibold text-gray-900 dark:text-white">"John Doe"</p>
                                    <p class="text-xs text-gray-500 dark:text-gray-400">"john.doe@example.com"</p>
                                </div>
                            </div>
                            <span class="px-3 py-1 bg-green-100 dark:bg-green-900/30 text-green-700 dark:text-green-400 text-xs font-semibold rounded-full">"ACTIVE"</span>
                        </div>

                        // Contact 2
                        <div class="flex items-center justify-between p-4 bg-gray-50 dark:bg-zinc-700/50 rounded-lg">
                            <div class="flex items-center gap-3">
                                <div class="w-10 h-10 rounded-full bg-blue-100 dark:bg-blue-900/30 flex items-center justify-center">
                                    <svg class="w-5 h-5 text-blue-600 dark:text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M16 7a4 4 0 11-8 0 4 4 0 018 0zM12 14a7 7 0 00-7 7h14a7 7 0 00-7-7z"></path>
                                    </svg>
                                </div>
                                <div>
                                    <p class="text-sm font-semibold text-gray-900 dark:text-white">"Jane Smith"</p>
                                    <p class="text-xs text-gray-500 dark:text-gray-400">"jane.smith@example.com"</p>
                                </div>
                            </div>
                            <span class="px-3 py-1 bg-orange-100 dark:bg-orange-900/30 text-orange-700 dark:text-orange-400 text-xs font-semibold rounded-full">"PROSPECT"</span>
                        </div>

                        // Contact 3
                        <div class="flex items-center justify-between p-4 bg-gray-50 dark:bg-zinc-700/50 rounded-lg">
                            <div class="flex items-center gap-3">
                                <div class="w-10 h-10 rounded-full bg-blue-100 dark:bg-blue-900/30 flex items-center justify-center">
                                    <svg class="w-5 h-5 text-blue-600 dark:text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M16 7a4 4 0 11-8 0 4 4 0 018 0zM12 14a7 7 0 00-7 7h14a7 7 0 00-7-7z"></path>
                                    </svg>
                                </div>
                                <div>
                                    <p class="text-sm font-semibold text-gray-900 dark:text-white">"Bob Johnson"</p>
                                    <p class="text-xs text-gray-500 dark:text-gray-400">"bob.johnson@example.com"</p>
                                </div>
                            </div>
                            <span class="px-3 py-1 bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-400 text-xs font-semibold rounded-full">"CUSTOMER"</span>
                        </div>
                    </div>
                </div>

                // Sales Pipeline
                <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-sm p-6">
                    <h2 class="text-xl font-semibold text-gray-900 dark:text-white mb-6">"Sales Pipeline"</h2>

                    <div class="grid grid-cols-2 gap-4">
                        // Leads
                        <div class="p-4 bg-gray-50 dark:bg-zinc-700/50 rounded-lg">
                            <p class="text-sm font-medium text-gray-600 dark:text-gray-400 mb-2">"Leads"</p>
                            <p class="text-2xl font-bold text-gray-900 dark:text-white">"24"</p>
                            <p class="text-sm text-green-600 dark:text-green-400 font-medium mt-1">"$12,400"</p>
                        </div>

                        // Qualified
                        <div class="p-4 bg-gray-50 dark:bg-zinc-700/50 rounded-lg">
                            <p class="text-sm font-medium text-gray-600 dark:text-gray-400 mb-2">"Qualified"</p>
                            <p class="text-2xl font-bold text-gray-900 dark:text-white">"12"</p>
                            <p class="text-sm text-green-600 dark:text-green-400 font-medium mt-1">"$8,200"</p>
                        </div>

                        // Proposal
                        <div class="p-4 bg-gray-50 dark:bg-zinc-700/50 rounded-lg">
                            <p class="text-sm font-medium text-gray-600 dark:text-gray-400 mb-2">"Proposal"</p>
                            <p class="text-2xl font-bold text-gray-900 dark:text-white">"8"</p>
                            <p class="text-sm text-green-600 dark:text-green-400 font-medium mt-1">"$15,600"</p>
                        </div>

                        // Closed Won
                        <div class="p-4 bg-gray-50 dark:bg-zinc-700/50 rounded-lg">
                            <p class="text-sm font-medium text-gray-600 dark:text-gray-400 mb-2">"Closed Won"</p>
                            <p class="text-2xl font-bold text-gray-900 dark:text-white">"5"</p>
                            <p class="text-sm text-green-600 dark:text-green-400 font-medium mt-1">"$22,800"</p>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    }
}
