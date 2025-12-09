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
    // Base Path is /app/svelte_spa/
    const pathname = window.location.pathname;
    const pathSegments = pathname.split('/').filter(Boolean);
    
    // Expected segments: ['app', 'svelte_spa', 'dashboard' | 'counter']
    const pageName = pathSegments.length >= 3 && pathSegments[0] === 'app' && pathSegments[1] === 'svelte_spa' 
      ? pathSegments[2] 
      : 'dashboard';
    
    if (['dashboard', 'counter'].includes(pageName)) {
      currentPage = pageName;
    }

    // Handle browser back/forward
    window.addEventListener('popstate', () => {
      const pathname = window.location.pathname;
      const pathSegments = pathname.split('/').filter(Boolean);
      const pageName = pathSegments.length >= 3 && pathSegments[0] === 'app' && pathSegments[1] === 'svelte_spa' 
        ? pathSegments[2] 
        : 'dashboard';
      
      if (['dashboard', 'counter'].includes(pageName)) {
        currentPage = pageName;
      }
    });
  });

  function handlePageChange(page) {
    const newPath = `/app/svelte_spa/${page}`;
    window.history.pushState({}, '', newPath);
    currentPage = page;
  }
</script>

<div class="h-screen flex overflow-hidden {isDark ? 'bg-zinc-950 text-gray-100' : 'bg-gray-50 text-gray-900'}">
  <Navigation {currentPage} onPageChange={handlePageChange} {isDark} />
  
  <!-- Main Content -->
  <main class="flex-1 overflow-auto transition-all duration-300 relative w-full">
    {#if currentPage === 'dashboard'}
      <Dashboard {isDark} />
    {:else if currentPage === 'counter'}
      <Counter {isDark} />
    {:else}
      <Dashboard {isDark} />
    {/if}
  </main>
</div>
