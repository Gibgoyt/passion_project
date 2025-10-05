import { createSignal } from 'solid-js';

export default function SolidCounter() {
  const [count, setCount] = createSignal(0);

  const increment = () => setCount(count() + 1);
  const decrement = () => setCount(count() - 1);

  return (
    <div class="counter-container solid-counter">
      <h3>Solid Counter</h3>
      <div class="counter-display">
        <button onClick={decrement} class="btn">-</button>
        <span class="count">{count()}</span>
        <button onClick={increment} class="btn">+</button>
      </div>
    </div>
  );
}