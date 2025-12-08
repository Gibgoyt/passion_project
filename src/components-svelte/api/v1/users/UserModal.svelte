<script lang="ts">
    import './shim'; // Ensure process.env shim exists
    import { Endpoints } from '@ts-testing/endpoints/index.ts';
    import { SvelteForm } from '../../../../../libraries/runtime_validation/svelte/index.svelte.ts';
    import { required, minLength, pattern, isString } from '../../../../../libraries/runtime_validation/index.ts';
    import { untrack } from 'svelte';

    let { isOpen = false, mode = 'create', userId = '', initialData = { name: '', surname: '' }, onClose, onSuccess } = $props<{
        isOpen: boolean;
        mode: 'create' | 'update';
        userId?: string;
        initialData?: { name: string; surname: string };
        onClose: () => void;
        onSuccess: () => void;
    }>();

    let serverError = $state('');
    let isSubmitting = $state(false);

    // Initialize form with Svelte-optimized runtime validation
    // @ts-ignore
    const form = new SvelteForm({ name: initialData.name, surname: initialData.surname }, {
        name: [
            required("Name is required"),
            isString(),
            minLength(2, "Name must be at least 2 characters"),
            pattern(/^[a-zA-Z\s-]+$/, "Name can only contain letters, spaces, and hyphens")
        ],
        surname: [
            required("Surname is required"),
            isString(),
            minLength(2, "Surname must be at least 2 characters"),
            pattern(/^[a-zA-Z\s-]+$/, "Surname can only contain letters, spaces, and hyphens")
        ]
    });

    // Update form values when initialData changes (e.g. opening edit modal)
    $effect(() => {
        if (isOpen) {
            untrack(() => {
                if (initialData) {
                    form.reset(initialData);
                    serverError = '';
                }
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
            let result;
            if (mode === 'create') {
                result = await Endpoints._Api.V1.Users.POST(form.values.name, form.values.surname);
            } else {
                if (!userId) throw new Error("User ID is required for updates");
                result = await Endpoints._Api.V1.Users[':UserId'].PUT(userId, form.values.name, form.values.surname);
            }

            if (result.status === 201 || result.status === 200) {
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
        <div class="bg-white dark:bg-gray-800 rounded-lg shadow-xl max-w-md w-full p-6 relative">
            <button 
                onclick={onClose}
                class="absolute top-4 right-4 text-gray-400 hover:text-gray-600 dark:hover:text-gray-200"
                aria-label="Close"
            >
                <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path></svg>
            </button>

            <h2 class="text-2xl font-bold mb-6 text-gray-900 dark:text-gray-100">
                {mode === 'create' ? 'Create New User' : 'Update User'}
            </h2>

            {#if serverError}
                <div class="mb-4 p-3 bg-red-100 border border-red-400 text-red-700 rounded text-sm">
                    {serverError}
                </div>
            {/if}

            <form onsubmit={handleSubmit} class="space-y-4">
                <!-- Name Field -->
                <div>
                    <label for="name" class="block text-sm font-medium text-gray-700 dark:text-gray-300">Name</label>
                    <input
                        type="text"
                        id="name"
                        value={form.values.name}
                        oninput={(e) => form.setField('name', e.currentTarget.value)}
                        onblur={() => form.blurField('name')}
                        class={`mt-1 block w-full rounded-md shadow-sm sm:text-sm p-2 bg-gray-50 dark:bg-gray-700 dark:text-white border ${form.touched.name && form.errors.name ? 'border-red-500 focus:border-red-500 focus:ring-red-500' : 'border-gray-300 focus:border-indigo-500 focus:ring-indigo-500'}`}
                        placeholder="e.g. John"
                    />
                    {#if form.touched.name && form.errors.name}
                        <p class="mt-1 text-sm text-red-600 dark:text-red-400">{form.errors.name}</p>
                    {/if}
                </div>

                <!-- Surname Field -->
                <div>
                    <label for="surname" class="block text-sm font-medium text-gray-700 dark:text-gray-300">Surname</label>
                    <input
                        type="text"
                        id="surname"
                        value={form.values.surname}
                        oninput={(e) => form.setField('surname', e.currentTarget.value)}
                        onblur={() => form.blurField('surname')}
                        class={`mt-1 block w-full rounded-md shadow-sm sm:text-sm p-2 bg-gray-50 dark:bg-gray-700 dark:text-white border ${form.touched.surname && form.errors.surname ? 'border-red-500 focus:border-red-500 focus:ring-red-500' : 'border-gray-300 focus:border-indigo-500 focus:ring-indigo-500'}`}
                        placeholder="e.g. Doe"
                    />
                    {#if form.touched.surname && form.errors.surname}
                        <p class="mt-1 text-sm text-red-600 dark:text-red-400">{form.errors.surname}</p>
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
                        disabled={isSubmitting || (Object.values(form.errors).some(e => e) && Object.values(form.touched).some(t => t))}
                        class="px-4 py-2 border border-transparent rounded-md shadow-sm text-sm font-medium text-white bg-indigo-600 hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500 disabled:opacity-50 disabled:cursor-not-allowed"
                    >
                        {isSubmitting ? 'Saving...' : (mode === 'create' ? 'Create User' : 'Update User')}
                    </button>
                </div>
            </form>
        </div>
    </div>
{/if}
