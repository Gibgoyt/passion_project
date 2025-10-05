import type { Component, JSX } from 'solid-js'

interface SummaryCardProps {
  label: string
  value: string | number
  icon: JSX.Element
  isDark: boolean
}

export const SummaryCard: Component<SummaryCardProps> = (props) => {
  return (
    <div class={`p-6 rounded-lg ${props.isDark ? 'bg-zinc-800' : 'bg-white'} shadow-sm`}>
      <div class="flex items-center justify-between">
        <div>
          <p class={`text-sm ${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
            {props.label}
          </p>
          <p class="text-2xl font-bold">{props.value}</p>
        </div>
        <div class={`${props.isDark ? 'text-gray-400' : 'text-gray-600'}`}>
          {props.icon}
        </div>
      </div>
    </div>
  )
}