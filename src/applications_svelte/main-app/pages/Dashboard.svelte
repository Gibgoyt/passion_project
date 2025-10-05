<script>
  let { isDark } = $props();

  const summaryCards = [
    {
      label: 'Active Users',
      value: '342',
      icon: `<svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"></path>
        <circle cx="9" cy="7" r="4"></circle>
        <path d="M23 21v-2a4 4 0 0 0-3-3.87"></path>
        <path d="M16 3.13a4 4 0 0 1 0 7.75"></path>
      </svg>`
    },
    {
      label: 'Total Projects',
      value: '12',
      icon: `<svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <path d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z" />
      </svg>`
    },
    {
      label: 'Revenue',
      value: '$8,420',
      icon: `<svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <line x1="12" y1="1" x2="12" y2="23"></line>
        <path d="M17 5H9.5a3.5 3.5 0 0 0 0 7h5a3.5 3.5 0 0 1 0 7H6"></path>
      </svg>`
    },
    {
      label: 'Completion Rate',
      value: '94.2%',
      icon: `<svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <polyline points="20 6 9 17 4 12"></polyline>
      </svg>`
    }
  ];
</script>

<div class="p-8">
  <div class="mb-8">
    <h1 class="text-3xl font-bold mb-2">Dashboard</h1>
    <p class="{isDark ? 'text-gray-400' : 'text-gray-600'}">
      Welcome to your Svelte SPA dashboard
    </p>
  </div>

  <!-- Summary Cards -->
  <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
    {#each summaryCards as card}
      <div class="{isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'} border rounded-lg p-6 hover:shadow-lg transition-shadow">
        <div class="flex items-center justify-between mb-4">
          <span class="{isDark ? 'text-gray-400' : 'text-gray-600'} text-sm">{card.label}</span>
          <span class="{isDark ? 'text-blue-400' : 'text-blue-600'}">
            {@html card.icon}
          </span>
        </div>
        <div class="text-2xl font-bold">{card.value}</div>
      </div>
    {/each}
  </div>

  <!-- Additional Content -->
  <div class="grid grid-cols-1 lg:grid-cols-2 gap-6">
    <div class="{isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'} border rounded-lg p-6">
      <h2 class="text-xl font-semibold mb-4">Svelte SPA Features</h2>
      <ul class="space-y-3">
        <li class="flex items-center">
          <span class="w-2 h-2 bg-green-500 rounded-full mr-3"></span>
          <span class="{isDark ? 'text-gray-300' : 'text-gray-700'}">Reactive state management with $state</span>
        </li>
        <li class="flex items-center">
          <span class="w-2 h-2 bg-green-500 rounded-full mr-3"></span>
          <span class="{isDark ? 'text-gray-300' : 'text-gray-700'}">Compile-time optimizations</span>
        </li>
        <li class="flex items-center">
          <span class="w-2 h-2 bg-green-500 rounded-full mr-3"></span>
          <span class="{isDark ? 'text-gray-300' : 'text-gray-700'}">Client-side routing</span>
        </li>
        <li class="flex items-center">
          <span class="w-2 h-2 bg-green-500 rounded-full mr-3"></span>
          <span class="{isDark ? 'text-gray-300' : 'text-gray-700'}">Dark mode support</span>
        </li>
      </ul>
    </div>

    <div class="{isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'} border rounded-lg p-6">
      <h2 class="text-xl font-semibold mb-4">Recent Activity</h2>
      <div class="space-y-3">
        <div class="flex items-center justify-between py-2 border-b {isDark ? 'border-zinc-700' : 'border-gray-100'}">
          <span class="{isDark ? 'text-gray-300' : 'text-gray-700'}">New user registered</span>
          <span class="text-sm {isDark ? 'text-gray-500' : 'text-gray-500'}">2 min ago</span>
        </div>
        <div class="flex items-center justify-between py-2 border-b {isDark ? 'border-zinc-700' : 'border-gray-100'}">
          <span class="{isDark ? 'text-gray-300' : 'text-gray-700'}">Project deployed</span>
          <span class="text-sm {isDark ? 'text-gray-500' : 'text-gray-500'}">5 min ago</span>
        </div>
        <div class="flex items-center justify-between py-2">
          <span class="{isDark ? 'text-gray-300' : 'text-gray-700'}">Settings updated</span>
          <span class="text-sm {isDark ? 'text-gray-500' : 'text-gray-500'}">1 hour ago</span>
        </div>
      </div>
    </div>
  </div>
</div>