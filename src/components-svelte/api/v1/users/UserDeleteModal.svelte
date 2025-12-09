<script lang="ts">
    import './shim';
    import { Endpoints } from '@ts-testing/endpoints/index.ts';
    import { SvelteForm } from '../../../../../libraries/runtime_validation/svelte/index.svelte.ts';
    import { required } from '../../../../../libraries/runtime_validation/index.ts';
    import { untrack } from 'svelte';

    let { isOpen = false, userId = '', onClose, onSuccess } = $props<{
        isOpen: boolean;
        userId: string;
        onClose: () => void;
        onSuccess: () => void;
    }>();

    let serverError = $state('');
    let isSubmitting = $state(false);

    // Initialize form with Svelte-optimized runtime validation
    // We use a closure for the validation to access the current 'userId' prop
    const form = new SvelteForm({ confirmId: '' }, {
        confirmId: [
            required("Please type the User ID to confirm"),
            (val) => val === userId ? null : "User ID does not match"
        ]
    });

    // Reset form when modal opens
    $effect(() => {
        if (isOpen) {
            untrack(() => {
                form.reset({ confirmId: '' });
                serverError = '';
            });
        }
    });

    async function handleSubmit(e: Event) {
        e.preventDefault();
        
        if (!form.validateAll()) {
            return;
        }

        isSubmitting = true;
        serverError = '';

        try {
            // Call DELETE endpoint
            const result = await Endpoints._Api.V1.Users[':UserId'].DELETE(userId);

            if (result.status === 200) {
                onSuccess();
                onClose();
            } else {
                serverError = `Error: ${JSON.stringify(result.data)}`;
            }
        } catch (err: any) {
            serverError = err.message || "An unexpected error occurred";
        } finally {
            isSubmitting = false;
        }
    }
</script>

{#if isOpen}
    <div class="fixed inset-0 z-50 flex items-center justify-center bg-black bg-opacity-50 p-4 overflow-y-auto">
        <div class="bg-white dark:bg-gray-800 rounded-lg shadow-xl max-w-md w-full p-6 relative border-t-4 border-red-500">
            <button 
                onclick={onClose}
                class="absolute top-4 right-4 text-gray-400 hover:text-gray-600 dark:hover:text-gray-200"
                aria-label="Close"
            >
                <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path></svg>
            </button>

            <h2 class="text-2xl font-bold mb-4 text-gray-900 dark:text-gray-100 flex items-center gap-2">
                <svg class="w-6 h-6 text-red-500" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z"></path></svg>
                Delete User
            </h2>

            <p class="mb-4 text-gray-600 dark:text-gray-300">
                Are you sure you want to delete this user? This action cannot be undone.
            </p>

            <p class="mb-4 text-sm text-gray-500 dark:text-gray-400 bg-gray-50 dark:bg-gray-900 p-3 rounded border border-gray-200 dark:border-gray-700">
                Please type <span class="font-mono font-bold text-gray-800 dark:text-gray-200 select-all">{userId}</span> to confirm.
            </p>

            {#if serverError}
                <div class="mb-4 p-3 bg-red-100 border border-red-400 text-red-700 rounded text-sm">
                    {serverError}
                </div>
            {/if}

            <form onsubmit={handleSubmit} class="space-y-4">
                <!-- Confirmation Input -->
                <div>
                    <label for="confirmId" class="block text-sm font-medium text-gray-700 dark:text-gray-300">User ID</label>
                    <input
                        type="text"
                        id="confirmId"
                        value={form.values.confirmId}
                        oninput={(e) => form.setField('confirmId', e.currentTarget.value)}
                        onblur={() => form.blurField('confirmId')}
                        class={`mt-1 block w-full rounded-md shadow-sm sm:text-sm p-2 bg-gray-50 dark:bg-gray-700 dark:text-white border ${form.touched.confirmId && form.errors.confirmId ? 'border-red-500 focus:border-red-500 focus:ring-red-500' : 'border-gray-300 focus:border-indigo-500 focus:ring-indigo-500'}`}
                        placeholder="Paste User ID here"
                        autocomplete="off"
                    />
                    {#if form.touched.confirmId && form.errors.confirmId}
                        <p class="mt-1 text-sm text-red-600 dark:text-red-400">{form.errors.confirmId}</p>
                    {/if}
                </div>

                <div class="flex justify-end pt-4 space-x-3">
                    <button
                        type="button"
                        onclick={onClose}
                        class="px-4 py-2 border border-gray-300 rounded-md shadow-sm text-sm font-medium text-gray-700 bg-white hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500 dark:bg-gray-700 dark:text-gray-300 dark:border-gray-600 dark:hover:bg-gray-600"
                    >
                        Cancel
                    </button>
                    <button
                        type="submit"
                        disabled={isSubmitting || form.values.confirmId !== userId}
                        class="px-4 py-2 border border-transparent rounded-md shadow-sm text-sm font-medium text-white bg-red-600 hover:bg-red-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-red-500 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                    >
                        {isSubmitting ? 'Deleting...' : 'Delete User'}
                    </button>
                </div>
            </form>
        </div>
    </div>
{/if}
