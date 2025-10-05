import type { Component } from 'solid-js'
import { createSignal } from 'solid-js'

interface CounterPageProps {
  isDark: boolean
}

const CounterPage: Component<CounterPageProps> = (props) => {
  const [count, setCount] = createSignal(0)

  const increment = () => setCount(count() + 1)
  const decrement = () => setCount(count() - 1)
  const reset = () => setCount(0)

  return (
    <div class="p-8 h-full flex items-center justify-center">
      <div class="max-w-md mx-auto text-center">
        <div class={`p-8 rounded-xl ${props.isDark ? 'bg-zinc-800' : 'bg-white'} shadow-lg`}>
          <h1 class="text-3xl font-bold mb-8">SolidJS Counter</h1>
          
          <div class="mb-8">
            <div class={`text-6xl font-bold mb-4 ${props.isDark ? 'text-blue-400' : 'text-blue-600'}`}>
              {count()}
            </div>
            <p class={`${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
              Current count value
            </p>
          </div>

          <div class="flex gap-4 justify-center">
            <button
              onClick={decrement}
              class="px-6 py-3 bg-red-600 hover:bg-red-700 text-white font-semibold rounded-lg transition-colors"
            >
              -1
            </button>
            
            <button
              onClick={reset}
              class={`px-6 py-3 font-semibold rounded-lg transition-colors ${
                props.isDark 
                  ? 'bg-zinc-600 hover:bg-zinc-700 text-white' 
                  : 'bg-gray-200 hover:bg-gray-300 text-gray-700'
              }`}
            >
              Reset
            </button>
            
            <button
              onClick={increment}
              class="px-6 py-3 bg-green-600 hover:bg-green-700 text-white font-semibold rounded-lg transition-colors"
            >
              +1
            </button>
          </div>

          <div class="mt-8 space-y-2">
            <div class={`text-sm ${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
              <strong>Framework:</strong> SolidJS
            </div>
            <div class={`text-sm ${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
              <strong>State Management:</strong> createSignal()
            </div>
            <div class={`text-sm ${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
              <strong>Reactivity:</strong> Fine-grained updates
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}

export default CounterPage