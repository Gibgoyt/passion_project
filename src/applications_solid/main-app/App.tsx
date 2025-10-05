import {
  createSignal,
  createEffect,
  onMount
} from 'solid-js'
import type {
  Component
} from 'solid-js'
import "src/styles/global.css"
import Navigation, {
  type Page
} from './components/navigation'
import {
  MiddlewareProvider,
  useMiddleware,
  loggingMiddleware,
  authMiddleware
} from './middleware'
import DashboardPage from './pages/dashboard/index'
import CounterPage from './pages/counter/index'

const AppContent: Component = () => {
  const [currentPage, setCurrentPage] = createSignal<Page>('dashboard')
  const [isDark, setIsDark] = createSignal(false)
  const middleware = useMiddleware()

  // Detect theme from localStorage and DOM class (shared with Astro app)
  onMount(() => {
    if (typeof window === 'undefined') return
    
    const isDarkMode = localStorage.getItem('darkMode') === 'true' || 
      (!localStorage.getItem('darkMode') && window.matchMedia('(prefers-color-scheme: dark)').matches) ||
      document.documentElement.classList.contains('dark')
    
    setIsDark(isDarkMode)
    
    // Initialize page from URL pathname
    const pathname = window.location.pathname
    const pathSegments = pathname.split('/').filter(Boolean)
    // Extract page name from /solid-spa/pagename structure
    const pageName = pathSegments.length >= 2 && pathSegments[0] === 'solid-spa' 
      ? pathSegments[1] 
      : 'dashboard'
    
    if (['dashboard', 'counter'].includes(pageName)) {
      setCurrentPage(pageName as Page)
    }
  })

  // Handle page changes with middleware navigation
  const handlePageChange = (page: Page) => {
    const newPath = `/solid-spa/${page}`
    middleware.navigate(newPath)
    setCurrentPage(page)
  }

  // Listen to route changes
  createEffect(() => {
    const currentRoute = middleware.currentRoute()
    const pathSegments = currentRoute.split('/').filter(Boolean)
    // Extract page name from /solid-spa/pagename structure
    const pageName = pathSegments.length >= 2 && pathSegments[0] === 'solid-spa' 
      ? pathSegments[1] 
      : 'dashboard'
    
    if (['dashboard', 'counter'].includes(pageName)) {
      setCurrentPage(pageName as Page)
    }
  })

  const updateTheme = (dark: boolean) => {
    setIsDark(dark)
    localStorage.setItem('darkMode', dark.toString())
    if (dark) {
      document.documentElement.classList.add('dark')
    } else {
      document.documentElement.classList.remove('dark')
    }
  }

  const renderPage = () => {
    const page = currentPage()
    switch (page) {
      case 'dashboard':
        return <DashboardPage isDark={isDark()} />
      case 'counter':
        return <CounterPage isDark={isDark()} />
      default:
        return <DashboardPage isDark={isDark()} />
    }
  }

  // Setup middleware
  onMount(() => {
    // Check authentication immediately on app load
    const authCheck = authMiddleware()
    const currentRoute = middleware.currentRoute()
    authCheck('', currentRoute) // Check current route on load
    
    // Add authentication middleware - this will redirect if not authenticated
    middleware.beforeNavigate(authMiddleware())
    
    // Add logging middleware
    middleware.afterNavigate(loggingMiddleware())
    
    // Example: Add custom middleware
    middleware.beforeNavigate((from, to) => {
      console.log(`Navigating from ${from} to ${to}`)
      return true // Allow navigation
    })
  })

  return (
    <div class={`h-screen flex overflow-hidden ${isDark() ? 'bg-zinc-900 text-gray-100' : 'bg-gray-50 text-gray-900'}`}>
      <Navigation 
        currentPage={currentPage()} 
        onPageChange={handlePageChange} 
        isDark={isDark()} 
      />
      
      {/* Main Content */}
      <main class={`flex-1 overflow-auto ml-64 transition-all duration-300`}>
        {renderPage()}
      </main>
    </div>
  )
}

const App: Component = () => {
  return (
    <MiddlewareProvider>
      <AppContent />
    </MiddlewareProvider>
  )
}

export default App