<script lang="ts">
    import './shim';
    import { Endpoints } from '@ts-testing/endpoints/index.ts';

    let { isOpen = false, userId = '', onClose } = $props<{
        isOpen: boolean;
        userId?: string;
        onClose: () => void;
    }>();

    let user = $state<any>(null);
    let loading = $state(false);
    let error = $state('');

    $effect(() => {
        if (isOpen && userId) {
            fetchUser();
        } else {
            user = null;
        }
    });

    async function fetchUser() {
        if (!userId) return;
        loading = true;
        error = '';
        try {
            const result = await Endpoints._Api.V1.Users[':UserId'].GET(userId);
            if (result.status === 200) {
                user = result.data;
            } else {
                error = `Failed to load user. Status: ${result.status}`;
            }
        } catch (e: any) {
            error = e.message;
        } finally {
            loading = false;
        }
    }
</script>

{#if isOpen}
    <div class="fixed inset-0 z-50 flex items-center justify-center bg-black bg-opacity-50 p-4 overflow-y-auto">
        <div class="bg-white dark:bg-gray-800 rounded-lg shadow-xl max-w-lg w-full p-6 relative">
            <button 
                onclick={onClose}
                class="absolute top-4 right-4 text-gray-400 hover:text-gray-600 dark:hover:text-gray-200"
                aria-label="Close"
            >
                <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path></svg>
            </button>

            <h2 class="text-2xl font-bold mb-6 text-gray-900 dark:text-gray-100">User Details</h2>

            {#if loading}
                <div class="flex justify-center py-8">
                     <svg class="animate-spin h-8 w-8 text-indigo-600" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                        <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
                        <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                    </svg>
                </div>
            {:else if error}
                <div class="p-4 bg-red-100 text-red-700 rounded mb-4">
                    {error}
                </div>
            {:else if user}
                <div class="space-y-4">
                    <div class="grid grid-cols-3 gap-4 border-b border-gray-200 dark:border-gray-700 pb-2">
                        <span class="font-medium text-gray-500 dark:text-gray-400">ID:</span>
                        <span class="col-span-2 text-gray-900 dark:text-gray-100 font-mono text-sm">{user.id}</span>
                    </div>
                    <div class="grid grid-cols-3 gap-4 border-b border-gray-200 dark:border-gray-700 pb-2">
                        <span class="font-medium text-gray-500 dark:text-gray-400">Name:</span>
                        <span class="col-span-2 text-gray-900 dark:text-gray-100">{user.name}</span>
                    </div>
                    <div class="grid grid-cols-3 gap-4 border-b border-gray-200 dark:border-gray-700 pb-2">
                        <span class="font-medium text-gray-500 dark:text-gray-400">Surname:</span>
                        <span class="col-span-2 text-gray-900 dark:text-gray-100">{user.surname}</span>
                    </div>
                    <div class="grid grid-cols-3 gap-4 border-b border-gray-200 dark:border-gray-700 pb-2">
                        <span class="font-medium text-gray-500 dark:text-gray-400">Created At:</span>
                        <span class="col-span-2 text-gray-900 dark:text-gray-100">{new Date(user.createdAt).toLocaleString()}</span>
                    </div>
                    <div class="grid grid-cols-3 gap-4">
                        <span class="font-medium text-gray-500 dark:text-gray-400">Updated At:</span>
                        <span class="col-span-2 text-gray-900 dark:text-gray-100">{new Date(user.updatedAt).toLocaleString()}</span>
                    </div>
                </div>
            {:else}
                <p class="text-gray-500">No user data found.</p>
            {/if}
            
            <div class="mt-6 flex justify-end">
                 <button
                    onclick={onClose}
                    class="px-4 py-2 bg-indigo-600 text-white rounded hover:bg-indigo-700 transition-colors"
                >
                    Close
                </button>
            </div>
        </div>
    </div>
{/if}
