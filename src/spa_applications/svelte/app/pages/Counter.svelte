<script>
  let { isDark } = $props();
  let count = $state(0);
</script>

<div class="h-full flex flex-col items-center justify-center p-8">
  <div class="max-w-md w-full">
    <div class="text-center mb-10">
      <h1 class="text-4xl font-extrabold tracking-tight mb-4 {isDark ? 'text-white' : 'text-gray-900'}">
        Interactive Counter
      </h1>
      <p class="text-lg {isDark ? 'text-zinc-400' : 'text-gray-500'}">
        A simple demonstration of Svelte 5 state management.
      </p>
    </div>
    
    <div class="rounded-2xl shadow-xl overflow-hidden border transition-colors
      {isDark ? 'bg-zinc-900 border-zinc-800' : 'bg-white border-gray-100'}">
      
      <!-- Counter Display -->
      <div class="py-16 text-center border-b {isDark ? 'border-zinc-800 bg-zinc-800/50' : 'border-gray-100 bg-gray-50/50'}">
        <span class="text-8xl font-black tabular-nums tracking-tighter
          {count > 0 ? 'text-indigo-600 dark:text-indigo-400' : count < 0 ? 'text-red-600 dark:text-red-400' : 'text-gray-400 dark:text-zinc-600'}">
          {count}
        </span>
        <p class="mt-4 text-sm uppercase tracking-widest font-semibold {isDark ? 'text-zinc-500' : 'text-gray-400'}">
          Current Count
        </p>
      </div>

      <!-- Controls -->
      <div class="p-8">
        <div class="grid grid-cols-3 gap-4">
          <button 
            onclick={() => count--}
            class="flex items-center justify-center py-4 rounded-xl font-bold text-lg transition-all duration-200 focus:outline-none focus:ring-4 focus:ring-red-500/20 active:scale-95
            {isDark 
              ? 'bg-zinc-800 hover:bg-zinc-700 text-red-400' 
              : 'bg-red-50 hover:bg-red-100 text-red-600'}"
            aria-label="Decrease count"
          >
            <svg class="w-6 h-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="3" d="M20 12H4" />
            </svg>
          </button>
          
          <button 
            onclick={() => count = 0}
            class="flex items-center justify-center py-4 rounded-xl font-bold text-lg transition-all duration-200 focus:outline-none focus:ring-4 focus:ring-gray-500/20 active:scale-95
            {isDark 
              ? 'bg-zinc-800 hover:bg-zinc-700 text-zinc-400' 
              : 'bg-gray-100 hover:bg-gray-200 text-gray-500'}"
            aria-label="Reset count"
          >
            <svg class="w-6 h-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15" />
            </svg>
          </button>

          <button 
            onclick={() => count++}
            class="flex items-center justify-center py-4 rounded-xl font-bold text-lg transition-all duration-200 shadow-lg shadow-indigo-500/30 focus:outline-none focus:ring-4 focus:ring-indigo-500/20 active:scale-95
            bg-indigo-600 hover:bg-indigo-700 text-white"
            aria-label="Increase count"
          >
            <svg class="w-6 h-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="3" d="M12 4v16m8-8H4" />
            </svg>
          </button>
        </div>
      </div>
    </div>
  </div>
</div>

