<script>
  let { isDark } = $props();

  const stats = [
    { 
      label: 'Total Users', 
      value: '12,345', 
      change: '+12%', 
      trend: 'up',
      icon: 'M12 4.354a4 4 0 110 5.292M15 21H3v-1a6 6 0 0112 0v1zm0 0h6v-1a6 6 0 00-9-5.197M13 7a4 4 0 11-8 0 4 4 0 018 0z'
    },
    { 
      label: 'Total Revenue', 
      value: '$45,231', 
      change: '+8.2%', 
      trend: 'up',
      icon: 'M12 8c-1.657 0-3 .895-3 2s1.343 2 3 2 3 .895 3 2-1.343 2-3 2m0-8c1.11 0 2.08.402 2.599 1M12 8V7m0 1v8m0 0v1m0-1c-1.11 0-2.08-.402-2.599-1M21 12a9 9 0 11-18 0 9 9 0 0118 0z'
    },
    { 
      label: 'Active Sessions', 
      value: '1,234', 
      change: '-3%', 
      trend: 'down',
      icon: 'M13 10V3L4 14h7v7l9-11h-7z'
    },
  ];
</script>

<div class="p-8">
  <div class="max-w-7xl mx-auto">
    <!-- Header -->
    <div class="flex flex-col sm:flex-row sm:items-center justify-between gap-4 mb-8">
      <div>
        <h1 class="text-3xl font-bold tracking-tight {isDark ? 'text-white' : 'text-gray-900'}">Dashboard</h1>
        <p class="mt-1 text-sm {isDark ? 'text-zinc-400' : 'text-gray-500'}">Overview of your application's performance.</p>
      </div>
      <div class="flex gap-3">
        <button class="px-4 py-2 rounded-lg text-sm font-medium border shadow-sm transition-colors
          {isDark ? 'bg-zinc-800 border-zinc-700 text-zinc-300 hover:bg-zinc-700' : 'bg-white border-gray-300 text-gray-700 hover:bg-gray-50'}">
          Last 30 Days
        </button>
        <button class="px-4 py-2 rounded-lg bg-indigo-600 hover:bg-indigo-700 text-white text-sm font-medium shadow-sm transition-colors flex items-center gap-2">
          <svg class="w-4 h-4" fill="none" viewBox="0 0 24 24" stroke="currentColor">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
          </svg>
          Export Report
        </button>
      </div>
    </div>

    <!-- Stats Grid -->
    <div class="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
      {#each stats as stat}
        <div class="p-6 rounded-xl border shadow-sm transition-all duration-200 hover:shadow-md
          {isDark ? 'bg-zinc-900 border-zinc-800' : 'bg-white border-gray-200'}">
          <div class="flex items-start justify-between">
            <div>
              <p class="text-sm font-medium {isDark ? 'text-zinc-400' : 'text-gray-500'}">{stat.label}</p>
              <h3 class="mt-2 text-3xl font-bold {isDark ? 'text-white' : 'text-gray-900'}">{stat.value}</h3>
            </div>
            <div class="p-2 rounded-lg {isDark ? 'bg-zinc-800 text-zinc-400' : 'bg-gray-50 text-gray-500'}">
              <svg class="w-6 h-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d={stat.icon} />
              </svg>
            </div>
          </div>
          <div class="mt-4 flex items-center text-sm">
            <span class="flex items-center font-medium
              {stat.trend === 'up' 
                ? 'text-green-500' 
                : 'text-red-500'}">
              {#if stat.trend === 'up'}
                <svg class="w-4 h-4 mr-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 7h8m0 0v8m0-8l-8 8-4-4-6 6" />
                </svg>
              {:else}
                <svg class="w-4 h-4 mr-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 17h8m0 0V9m0 8l-8-8-4 4-6-6" />
                </svg>
              {/if}
              {stat.change}
            </span>
            <span class="ml-2 {isDark ? 'text-zinc-500' : 'text-gray-400'}">from last month</span>
          </div>
        </div>
      {/each}
    </div>

    <!-- Chart Section -->
    <div class="p-6 rounded-xl border shadow-sm {isDark ? 'bg-zinc-900 border-zinc-800' : 'bg-white border-gray-200'}">
      <div class="flex items-center justify-between mb-6">
        <h3 class="text-lg font-semibold {isDark ? 'text-white' : 'text-gray-900'}">User Activity</h3>
        <select class="text-sm rounded-md border-none ring-1 ring-inset px-3 py-1.5 focus:ring-2 focus:ring-indigo-600 outline-none
          {isDark ? 'bg-zinc-800 ring-zinc-700 text-zinc-300' : 'bg-gray-50 ring-gray-200 text-gray-700'}">
          <option>Last 7 days</option>
          <option>Last 30 days</option>
          <option>Last year</option>
        </select>
      </div>
      
      <!-- CSS-only Bar Chart -->
      <div class="h-80 flex items-end justify-between gap-4 pt-4">
        {#each Array(12) as _, i}
          {@const height = Math.floor(Math.random() * (100 - 20 + 1) + 20)}
          <div class="w-full h-full flex flex-col justify-end group cursor-pointer relative">
             <!-- Tooltip -->
            <div class="absolute -top-10 left-1/2 -translate-x-1/2 opacity-0 group-hover:opacity-100 transition-opacity pointer-events-none mb-2 z-10">
               <div class="px-2 py-1 rounded bg-gray-900 text-white text-xs whitespace-nowrap shadow-lg">
                 Value: {height}%
               </div>
            </div>
            
            <div class="w-full bg-indigo-500/10 rounded-t-sm relative overflow-hidden transition-all duration-300 group-hover:bg-indigo-500/20" 
                 style="height: {height}%">
              <div class="absolute bottom-0 w-full bg-indigo-600 transition-all duration-500 group-hover:bg-indigo-500" style="height: 100%"></div>
            </div>
            <div class="mt-3 text-xs text-center {isDark ? 'text-zinc-500' : 'text-gray-400'}">
              {['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'][i]}
            </div>
          </div>
        {/each}
      </div>
    </div>
  </div>
</div>
