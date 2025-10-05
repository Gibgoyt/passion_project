import { component$, useSignal, $ } from '@builder.io/qwik';

export const QwikCounter = component$(() => {
  const count = useSignal(0);

  const increment = $(() => {
    count.value++;
  });

  const decrement = $(() => {
    count.value--;
  });

  return (
    <div class="counter-container qwik-counter">
      <h3>Qwik Counter</h3>
      <div class="counter-display">
        <button onClick$={decrement} class="btn">-</button>
        <span class="count">{count.value}</span>
        <button onClick$={increment} class="btn">+</button>
      </div>
    </div>
  );
});

export default QwikCounter;