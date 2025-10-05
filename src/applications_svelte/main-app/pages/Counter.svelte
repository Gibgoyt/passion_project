<script>
  let { isDark } = $props();
  
  let count = $state(0);
  let step = $state(1);
  let history = $state([]);

  function increment() {
    count += step;
    history = [...history, { action: 'increment', value: step, result: count }];
  }

  function decrement() {
    count -= step;
    history = [...history, { action: 'decrement', value: step, result: count }];
  }

  function reset() {
    count = 0;
    history = [...history, { action: 'reset', value: 0, result: 0 }];
  }

  function setStep(value) {
    step = parseInt(value) || 1;
  }
</script>

<div class="p-8">
  <div class="mb-8">
    <h1 class="text-3xl font-bold mb-2">Counter</h1>
    <p class="{isDark ? 'text-gray-400' : 'text-gray-600'}">
      Interactive counter demonstrating Svelte's reactivity
    </p>
  </div>

  <div class="grid grid-cols-1 lg:grid-cols-2 gap-8">
    <!-- Counter Controls -->
    <div class="{isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'} border rounded-lg p-8">
      <div class="text-center mb-8">
        <div class="text-6xl font-bold mb-4 {isDark ? 'text-blue-400' : 'text-blue-600'}">
          {count}
        </div>
        <p class="{isDark ? 'text-gray-400' : 'text-gray-600'}">Current Count</p>
      </div>

      <div class="mb-6">
        <label class="block mb-2 {isDark ? 'text-gray-300' : 'text-gray-700'}">
          Step Size
        </label>
        <input 
          type="number" 
          value={step}
          oninput={(e) => setStep(e.target.value)}
          min="1"
          class="w-full px-4 py-2 rounded-lg border {isDark ? 'bg-zinc-700 border-zinc-600 text-gray-100' : 'bg-white border-gray-300 text-gray-900'}"
        />
      </div>

      <div class="flex justify-center space-x-4">
        <button 
          onclick={decrement}
          class="px-6 py-3 bg-red-600 text-white rounded-lg hover:bg-red-700 transition-colors font-medium"
        >
          -{step}
        </button>
        
        <button 
          onclick={reset}
          class="px-6 py-3 {isDark ? 'bg-zinc-600 hover:bg-zinc-700' : 'bg-gray-600 hover:bg-gray-700'} text-white rounded-lg transition-colors font-medium"
        >
          Reset
        </button>
        
        <button 
          onclick={increment}
          class="px-6 py-3 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors font-medium"
        >
          +{step}
        </button>
      </div>
    </div>

    <!-- History -->
    <div class="{isDark ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'} border rounded-lg p-8">
      <h2 class="text-xl font-semibold mb-4">History</h2>
      <div class="space-y-2 max-h-96 overflow-y-auto">
        {#if history.length === 0}
          <p class="{isDark ? 'text-gray-500' : 'text-gray-400'}">No actions yet</p>
        {:else}
          {#each history.slice().reverse() as item, i}
            <div class="flex items-center justify-between py-2 px-3 rounded {isDark ? 'bg-zinc-700' : 'bg-gray-50'}">
              <span class="capitalize {item.action === 'increment' ? 'text-green-600' : item.action === 'decrement' ? 'text-red-600' : 'text-gray-600'}">
                {item.action}
              </span>
              <span class="{isDark ? 'text-gray-300' : 'text-gray-700'}">
                {item.action === 'reset' ? 'Reset to 0' : `${item.action === 'increment' ? '+' : '-'}${item.value} = ${item.result}`}
              </span>
            </div>
          {/each}
        {/if}
      </div>
    </div>
  </div>

  <!-- Features -->
  <div class="mt-8 {isDark ? 'bg-blue-900/20 border-blue-800' : 'bg-blue-50 border-blue-200'} border rounded-lg p-6">
    <h3 class="text-lg font-semibold mb-3 {isDark ? 'text-blue-300' : 'text-blue-800'}">
      Svelte Counter Features
    </h3>
    <ul class="grid grid-cols-1 md:grid-cols-2 gap-3 {isDark ? 'text-blue-200' : 'text-blue-700'}">
      <li class="flex items-center">
        <span class="w-2 h-2 bg-blue-500 rounded-full mr-3"></span>
        Reactive state with $state() rune
      </li>
      <li class="flex items-center">
        <span class="w-2 h-2 bg-blue-500 rounded-full mr-3"></span>
        Two-way data binding
      </li>
      <li class="flex items-center">
        <span class="w-2 h-2 bg-blue-500 rounded-full mr-3"></span>
        Event handling with onclick
      </li>
      <li class="flex items-center">
        <span class="w-2 h-2 bg-blue-500 rounded-full mr-3"></span>
        Automatic DOM updates
      </li>
    </ul>
  </div>
</div>