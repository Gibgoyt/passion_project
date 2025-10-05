import { createSignal } from 'solid-js';

export default function Counter() {
  const [count, setCount] = createSignal(0);

  return (
    <div class="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6 max-w-md mx-auto">
      <h2 class="text-2xl font-semibold mb-4 text-center">SolidJS Counter Demo</h2>
      <div class="text-center">
        <p class="text-4xl font-bold mb-6 text-blue-600 dark:text-blue-400">
          {count()}
        </p>
        <div class="flex gap-4 justify-center">
          <button
            onClick={() => setCount(count() - 1)}
            class="px-4 py-2 bg-red-500 text-white rounded-lg hover:bg-red-600 transition-colors"
          >
            Decrement
          </button>
          <button
            onClick={() => setCount(0)}
            class="px-4 py-2 bg-gray-500 text-white rounded-lg hover:bg-gray-600 transition-colors"
          >
            Reset
          </button>
          <button
            onClick={() => setCount(count() + 1)}
            class="px-4 py-2 bg-green-500 text-white rounded-lg hover:bg-green-600 transition-colors"
          >
            Increment
          </button>
        </div>
      </div>
    </div>
  );
}