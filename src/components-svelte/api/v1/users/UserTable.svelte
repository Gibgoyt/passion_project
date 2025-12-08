<script lang="ts">
    import './shim';
    import { onMount } from 'svelte';
    import { Endpoints } from '@ts-testing/endpoints/index.ts';
    import UserModal from './UserModal.svelte';
    import UserDetailModal from './UserDetailModal.svelte';

    let users = $state<any[]>([]);
    let loading = $state(true);
    let error = $state('');

    // Modal States
    let isCreateModalOpen = $state(false);
    let isEditModalOpen = $state(false);
    let isDetailModalOpen = $state(false);
    
    let selectedUserId = $state('');
    let selectedUserData = $state({ name: '', surname: '' });

    onMount(() => {
        loadUsers();
    });

    async function loadUsers() {
        loading = true;
        error = '';
        try {
            const result = await Endpoints._Api.V1.Users.GET();
            if (result.status === 200) {
                users = result.data;
            } else {
                error = `Failed to load users. Status: ${result.status}`;
            }
        } catch (e: any) {
            error = e.message;
        } finally {
            loading = false;
        }
    }

    function openCreateModal() {
        isCreateModalOpen = true;
    }

    function openEditModal(user: any) {
        selectedUserId = user.id;
        selectedUserData = { name: user.name, surname: user.surname };
        isEditModalOpen = true;
    }

    function openDetailModal(id: string) {
        selectedUserId = id;
        isDetailModalOpen = true;
    }

    function handleSuccess() {
        loadUsers();
    }
</script>

<div class="bg-white dark:bg-gray-800 shadow-md rounded-lg overflow-hidden border border-gray-200 dark:border-gray-700">
    <!-- Header -->
    <div class="px-6 py-4 border-b border-gray-200 dark:border-gray-700 flex justify-between items-center bg-gray-50 dark:bg-gray-900">
        <h2 class="text-xl font-semibold text-gray-800 dark:text-gray-200">Users Management</h2>
        <button
            onclick={openCreateModal}
            class="px-4 py-2 bg-green-600 hover:bg-green-700 text-white text-sm font-medium rounded-md shadow-sm transition-colors flex items-center gap-2"
        >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4"></path></svg>
            Add New User
        </button>
    </div>

    <!-- Content -->
    <div class="p-6">
        {#if loading}
            <div class="flex justify-center py-10">
                <svg class="animate-spin h-10 w-10 text-indigo-600" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                    <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
                    <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                </svg>
            </div>
        {:else if error}
            <div class="bg-red-100 border border-red-400 text-red-700 px-4 py-3 rounded relative" role="alert">
                <strong class="font-bold">Error!</strong>
                <span class="block sm:inline">{error}</span>
            </div>
        {:else if users.length === 0}
            <div class="text-center py-10 text-gray-500 dark:text-gray-400">
                No users found. Create one to get started.
            </div>
        {:else}
            <div class="overflow-x-auto">
                <table class="min-w-full divide-y divide-gray-200 dark:divide-gray-700">
                    <thead class="bg-gray-50 dark:bg-gray-800">
                        <tr>
                            <th scope="col" class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">Name</th>
                            <th scope="col" class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">Surname</th>
                            <th scope="col" class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">Created At</th>
                            <th scope="col" class="px-6 py-3 text-right text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">Actions</th>
                        </tr>
                    </thead>
                    <tbody class="bg-white dark:bg-gray-900 divide-y divide-gray-200 dark:divide-gray-700">
                        {#each users as user (user.id)}
                            <tr class="hover:bg-gray-50 dark:hover:bg-gray-800 transition-colors">
                                <td class="px-6 py-4 whitespace-nowrap text-sm font-medium text-gray-900 dark:text-white">{user.name}</td>
                                <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-500 dark:text-gray-300">{user.surname}</td>
                                <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-500 dark:text-gray-400">{new Date(user.createdAt).toLocaleDateString()}</td>
                                <td class="px-6 py-4 whitespace-nowrap text-right text-sm font-medium space-x-2">
                                    <button 
                                        onclick={() => openDetailModal(user.id)}
                                        class="text-indigo-600 hover:text-indigo-900 dark:text-indigo-400 dark:hover:text-indigo-300 transition-colors"
                                    >
                                        View
                                    </button>
                                    <span class="text-gray-300">|</span>
                                    <button 
                                        onclick={() => openEditModal(user)}
                                        class="text-blue-600 hover:text-blue-900 dark:text-blue-400 dark:hover:text-blue-300 transition-colors"
                                    >
                                        Edit
                                    </button>
                                </td>
                            </tr>
                        {/each}
                    </tbody>
                </table>
            </div>
        {/if}
    </div>
</div>

<!-- Modals -->
<UserModal 
    isOpen={isCreateModalOpen} 
    mode="create" 
    onClose={() => isCreateModalOpen = false} 
    onSuccess={handleSuccess} 
/>

<UserModal 
    isOpen={isEditModalOpen} 
    mode="update" 
    userId={selectedUserId}
    initialData={selectedUserData}
    onClose={() => isEditModalOpen = false} 
    onSuccess={handleSuccess} 
/>

<UserDetailModal 
    isOpen={isDetailModalOpen} 
    userId={selectedUserId} 
    onClose={() => isDetailModalOpen = false} 
/>
