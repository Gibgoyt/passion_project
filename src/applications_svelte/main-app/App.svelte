<script>
  import { onMount } from 'svelte';
  import Navigation from './components/Navigation.svelte';
  import Dashboard from './pages/Dashboard.svelte';
  import Counter from './pages/Counter.svelte';
  
  let currentPage = $state('dashboard');
  let isDark = $state(false);

  // Simple client-side routing
  onMount(() => {
    // Detect theme from localStorage and DOM class
    const isDarkMode = localStorage.getItem('darkMode') === 'true' || 
      (!localStorage.getItem('darkMode') && window.matchMedia('(prefers-color-scheme: dark)').matches) ||
      document.documentElement.classList.contains('dark');
    
    isDark = isDarkMode;
    
    // Initialize page from URL pathname
    const pathname = window.location.pathname;
    const pathSegments = pathname.split('/').filter(Boolean);
    const pageName = pathSegments.length >= 2 && pathSegments[0] === 'svelte-spa' 
      ? pathSegments[1] 
      : 'dashboard';
    
    if (['dashboard', 'counter'].includes(pageName)) {
      currentPage = pageName;
    }

    // Handle browser back/forward
    window.addEventListener('popstate', () => {
      const pathname = window.location.pathname;
      const pathSegments = pathname.split('/').filter(Boolean);
      const pageName = pathSegments.length >= 2 && pathSegments[0] === 'svelte-spa' 
        ? pathSegments[1] 
        : 'dashboard';
      
      if (['dashboard', 'counter'].includes(pageName)) {
        currentPage = pageName;
      }
    });
  });

  function handlePageChange(page) {
    const newPath = `/svelte-spa/${page}`;
    window.history.pushState({}, '', newPath);
    currentPage = page;
  }
</script>

<div class="h-screen flex overflow-hidden {isDark ? 'bg-zinc-900 text-gray-100' : 'bg-gray-50 text-gray-900'}">
  <Navigation {currentPage} onPageChange={handlePageChange} {isDark} />
  
  <!-- Main Content -->
  <main class="flex-1 overflow-auto ml-64 transition-all duration-300">
    {#if currentPage === 'dashboard'}
      <Dashboard {isDark} />
    {:else if currentPage === 'counter'}
      <Counter {isDark} />
    {:else}
      <Dashboard {isDark} />
    {/if}
  </main>
</div>