<script>
  let { currentPage, onPageChange, isDark } = $props();

  const navItems = [
    { id: 'dashboard', label: 'Dashboard', icon: 'M4 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2V6zM14 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2V6zM4 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2v-2zM14 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2v-2z' },
    { id: 'counter', label: 'Counter', icon: 'M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z' }
  ];
</script>

<aside class="w-64 fixed inset-y-0 left-0 z-50 flex flex-col transition-all duration-300 border-r {isDark ? 'bg-zinc-900 border-zinc-800' : 'bg-white border-gray-200'}">
  <div class="h-16 flex items-center px-6 border-b {isDark ? 'border-zinc-800' : 'border-gray-200'}">
    <span class="text-xl font-bold bg-clip-text text-transparent bg-gradient-to-r from-indigo-500 to-purple-600">
      Svelte SPA
    </span>
  </div>

  <nav class="flex-1 px-4 py-6 space-y-1 overflow-y-auto">
    {#each navItems as item}
      <a
        href={`/app/svelte_spa/${item.id}`}
        onclick={(e) => { e.preventDefault(); onPageChange(item.id); }}
        class="flex items-center px-3 py-2.5 text-sm font-medium rounded-lg transition-colors group relative
        {currentPage === item.id 
          ? (isDark ? 'bg-indigo-500/10 text-indigo-400' : 'bg-indigo-50 text-indigo-700') 
          : (isDark ? 'text-zinc-400 hover:bg-zinc-800 hover:text-zinc-100' : 'text-gray-600 hover:bg-gray-50 hover:text-gray-900')}"
      >
        <svg class="w-5 h-5 mr-3 flex-shrink-0 {currentPage === item.id ? (isDark ? 'text-indigo-400' : 'text-indigo-600') : 'text-gray-400 group-hover:text-gray-500'}" fill="none" viewBox="0 0 24 24" stroke="currentColor">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d={item.icon} />
        </svg>
        {item.label}
        {#if currentPage === item.id}
          <div class="absolute right-0 top-1/2 -translate-y-1/2 w-1 h-8 bg-indigo-500 rounded-l-full"></div>
        {/if}
      </a>
    {/each}
  </nav>

  <div class="p-4 border-t {isDark ? 'border-zinc-800' : 'border-gray-200'}">
    <div class="flex items-center gap-3 px-3 py-2">
      <div class="w-8 h-8 rounded-full bg-gradient-to-tr from-indigo-500 to-purple-500 flex items-center justify-center text-white font-bold text-xs">
        US
      </div>
      <div class="flex-1 min-w-0">
        <p class="text-sm font-medium {isDark ? 'text-zinc-200' : 'text-gray-900'} truncate">
          User Account
        </p>
        <p class="text-xs {isDark ? 'text-zinc-500' : 'text-gray-500'} truncate">
          user@example.com
        </p>
      </div>
    </div>
  </div>
</aside>
