<script>
  let { isDark } = $props();

  const stats = [
    { label: 'Total Users', value: '12,345', change: '+12%', trend: 'up' },
    { label: 'Revenue', value: '$45,231', change: '+8.2%', trend: 'up' },
    { label: 'Active Sessions', value: '1,234', change: '-3%', trend: 'down' },
  ];
</script>

<div class="p-8">
  <div class="max-w-6xl mx-auto">
    <div class="flex justify-between items-center mb-8">
      <div>
        <h1 class="text-3xl font-bold {isDark ? 'text-white' : 'text-gray-900'}">Dashboard</h1>
        <p class="mt-2 {isDark ? 'text-zinc-400' : 'text-gray-500'}">Overview of your application statistics</p>
      </div>
      <button class="px-4 py-2 rounded-lg bg-indigo-600 hover:bg-indigo-700 text-white text-sm font-medium transition-colors">
        Download Report
      </button>
    </div>

    <div class="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
      {#each stats as stat}
        <div class="p-6 rounded-2xl border shadow-sm transition-all duration-200 hover:-translate-y-1
          {isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'}">
          <p class="text-sm font-medium {isDark ? 'text-zinc-400' : 'text-gray-500'}">{stat.label}</p>
          <div class="mt-4 flex items-baseline justify-between">
            <span class="text-3xl font-bold {isDark ? 'text-white' : 'text-gray-900'}">{stat.value}</span>
            <span class="text-sm font-medium px-2.5 py-0.5 rounded-full
              {stat.trend === 'up' 
                ? (isDark ? 'bg-green-500/10 text-green-400' : 'bg-green-100 text-green-700')
                : (isDark ? 'bg-red-500/10 text-red-400' : 'bg-red-100 text-red-700')}">
              {stat.change}
            </span>
          </div>
        </div>
      {/each}
    </div>

    <!-- Placeholder Chart Area -->
    <div class="p-6 rounded-2xl border shadow-sm {isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'}">
      <h3 class="text-lg font-semibold mb-6 {isDark ? 'text-white' : 'text-gray-900'}">Activity Overview</h3>
      <div class="h-64 flex items-end justify-between gap-2">
        {#each Array(12) as _, i}
          <div class="w-full bg-indigo-500/20 rounded-t-lg hover:bg-indigo-500/40 transition-colors relative group" style="height: {Math.random() * 100}%">
            <div class="absolute bottom-0 w-full bg-indigo-500 rounded-t-lg transition-all duration-500 group-hover:bg-indigo-400" style="height: 100%"></div>
          </div>
        {/each}
      </div>
    </div>
  </div>
</div>
