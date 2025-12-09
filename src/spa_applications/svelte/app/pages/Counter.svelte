<script>
  let { isDark } = $props();
  let count = $state(0);
</script>

<div class="p-8">
  <div class="max-w-4xl mx-auto">
    <h1 class="text-3xl font-bold mb-8 {isDark ? 'text-white' : 'text-gray-900'}">Counter Page</h1>
    
    <div class="rounded-2xl p-8 shadow-sm border {isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'}">
      <div class="text-center">
        <div class="text-6xl font-bold mb-8 font-mono {isDark ? 'text-indigo-400' : 'text-indigo-600'}">
          {count}
        </div>
        
        <div class="flex justify-center gap-4">
          <button 
            onclick={() => count--}
            class="px-6 py-3 rounded-xl font-semibold transition-all duration-200 
            {isDark 
              ? 'bg-zinc-700 hover:bg-zinc-600 text-white' 
              : 'bg-gray-100 hover:bg-gray-200 text-gray-900'}"
          >
            Decrease
          </button>
          
          <button 
            onclick={() => count = 0}
            class="px-6 py-3 rounded-xl font-semibold transition-all duration-200 text-gray-500 hover:text-gray-700 dark:hover:text-gray-300"
          >
            Reset
          </button>

          <button 
            onclick={() => count++}
            class="px-6 py-3 rounded-xl font-semibold transition-all duration-200 shadow-lg shadow-indigo-500/20
            bg-indigo-600 hover:bg-indigo-700 text-white"
          >
            Increase
          </button>
        </div>
      </div>
    </div>
  </div>
</div>
