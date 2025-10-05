/** @jsxImportSource @builder.io/qwik */
import { component$, useSignal, $, useVisibleTask$, useComputed$ } from '@builder.io/qwik'

interface AppProps {
  initialPath?: string
}

// Client-side middleware configuration
interface MiddlewareConfig {
  allowYouTubeAccess: boolean
  allowAnalytics: boolean
  userRole: 'admin' | 'user' | 'guest'
}

// Function to detect dark mode (can run during SSR)
const getInitialDarkMode = () => {
  if (typeof document === 'undefined') {
    // During SSR, default to dark mode to prevent flickering
    // (assuming most users prefer dark mode, adjust as needed)
    return true
  }

  return document.documentElement.classList.contains('dark') ||
    localStorage.getItem('darkMode') === 'true' ||
    (!localStorage.getItem('darkMode') && window.matchMedia('(prefers-color-scheme: dark)').matches)
}

// Middleware rules - this runs on the client
const checkRouteAccess = (path: string, config: MiddlewareConfig): { allowed: boolean; redirectTo?: string; message?: string } => {
  console.log(`QWIK MIDDLEWARE`)

  // Add more middleware rules as needed
  return {
    allowed: true
  }
}

export const App = component$<AppProps>(({ initialPath = '/' }) => {
  const currentPath = useSignal(initialPath)

  // Initialize with better dark mode detection
  const isDark = useSignal(getInitialDarkMode())

  const count = useSignal(0)

  // Middleware configuration
  const middlewareConfig = useSignal<MiddlewareConfig>({
    allowYouTubeAccess: true,
    allowAnalytics: false,
    userRole: 'user'
  })

  // Computed signal for current route access
  const routeAccess = useComputed$(() => {
    return checkRouteAccess(currentPath.value, middlewareConfig.value)
  })

  // Client-side navigation with middleware
  const navigate = $((path: string) => {
    const accessCheck = checkRouteAccess(path, middlewareConfig.value)

    if (!accessCheck.allowed) {
      console.log(`Access denied to ${path}:`, accessCheck.message)
      if (accessCheck.redirectTo) {
        currentPath.value = accessCheck.redirectTo
        if (typeof window !== 'undefined') {
          const fullPath = `/qwik-spa-auth${accessCheck.redirectTo}`
          window.history.pushState({}, '', fullPath)
        }
      }
      return
    }

    currentPath.value = path
    if (typeof window !== 'undefined') {
      const fullPath = `/qwik-spa-auth${path}`
      window.history.pushState({}, '', fullPath)
    }
  })

  // Initialize URL synchronization
  useVisibleTask$(() => {
    if (typeof window !== 'undefined') {
      const currentUrl = window.location.pathname
      const expectedPrefix = '/qwik-spa-auth'
      if (currentUrl.startsWith(expectedPrefix)) {
        const urlPath = currentUrl.slice(expectedPrefix.length) || '/'
        if (urlPath !== currentPath.value) {
          const accessCheck = checkRouteAccess(urlPath, middlewareConfig.value)
          if (accessCheck.allowed) {
            currentPath.value = urlPath
          } else if (accessCheck.redirectTo) {
            currentPath.value = accessCheck.redirectTo
            window.history.replaceState({}, '', `/qwik-spa-auth${accessCheck.redirectTo}`)
          }
        }
      }
    }
  })

  // Refined dark mode initialization - only run once and don't cause re-renders
  useVisibleTask$(() => {
    // Only update if the current state doesn't match what it should be
    const shouldBeDark = getInitialDarkMode()

    if (isDark.value !== shouldBeDark) {
      isDark.value = shouldBeDark
    }

    // Apply to document
    if (shouldBeDark) {
      document.documentElement.classList.add('dark')
    } else {
      document.documentElement.classList.remove('dark')
    }
  })

  const updateTheme = $((dark: boolean) => {
    isDark.value = dark
    localStorage.setItem('darkMode', dark.toString())
    if (dark) {
      document.documentElement.classList.add('dark')
    } else {
      document.documentElement.classList.remove('dark')
    }
  })

  const toggleTheme = $(() => {
    updateTheme(!isDark.value)
  })

  const increment = $(() => {
    count.value++
  })

  const decrement = $(() => {
    count.value--
  })

  const reset = $(() => {
    count.value = 0
  })

  const togglePermission = $((permission: keyof MiddlewareConfig) => {
    const current = middlewareConfig.value
    middlewareConfig.value = {
      ...current,
      [permission]: typeof current[permission] === 'boolean' ? !current[permission] : current[permission]
    }
  })

  // Render access denied UI if current route is not allowed
  if (!routeAccess.value.allowed) {
    return (
      <div class={`min-h-screen flex items-center justify-center ${isDark.value ? 'bg-zinc-900 text-gray-100' : 'bg-gradient-to-br from-blue-50 to-indigo-100'}`}>
        <div
          class={`max-w-md w-full mx-4 p-8 rounded-xl shadow-lg text-center ${isDark.value ? 'bg-zinc-800' : 'bg-white'}`}>
          <div class="w-16 h-16 bg-red-100 rounded-full flex items-center justify-center mb-4 mx-auto">
            <svg class="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2"
                    d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z"></path>
            </svg>
          </div>
          <h1 class={`text-2xl font-bold mb-2 ${isDark.value ? 'text-gray-100' : 'text-gray-900'}`}>Access Denied</h1>
          <p class={`mb-6 ${isDark.value ? 'text-gray-400' : 'text-gray-600'}`}>
            {routeAccess.value.message || 'You don\'t have permission to access this page.'}
          </p>
          <a
            href="/apps"
            class={`flex items-center gap-2 px-4 py-2 rounded-lg transition-colors ${
              isDark.value
                ? 'bg-zinc-700 hover:bg-zinc-600 text-gray-300'
                : 'bg-gray-100 hover:bg-gray-200 text-gray-700'
            }`}
          >
            <span>←</span>
            <span class="font-medium">Back to Apps</span>
          </a>
        </div>
      </div>
    )
  }

  return (
    <div
      class={`min-h-screen transition-colors ${isDark.value ? 'bg-zinc-900 text-gray-100' : 'bg-gradient-to-br from-blue-50 to-indigo-100'}`}>
      {/* Navigation */}
      <nav class={`shadow-lg border-b ${isDark.value ? 'bg-zinc-800 border-zinc-700' : 'bg-white border-gray-200'}`}>
        <div class="max-w-6xl mx-auto px-4">
          <div class="flex justify-between items-center py-4">
            <div class="flex items-center space-x-4">
              <h1 class={`text-2xl font-bold ${isDark.value ? 'text-gray-100' : 'text-gray-800'}`}>Qwik SPA</h1>
              <a
                href="/apps"
                class={`flex items-center gap-2 px-4 py-2 rounded-lg transition-colors ${
                  isDark.value
                    ? 'bg-zinc-700 hover:bg-zinc-600 text-gray-300'
                    : 'bg-gray-100 hover:bg-gray-200 text-gray-700'
                }`}
              >
                <span>←</span>
                <span class="font-medium">Back to Apps</span>
              </a>
            </div>

            <div class="flex items-center space-x-4">
              <div class="flex space-x-1">
                <button
                  onClick$={() => navigate('/dashboard')}
                  class={`px-6 py-2 rounded-lg font-medium transition-all duration-200 ${
                    currentPath.value === '/dashboard'
                      ? 'bg-blue-600 text-white shadow-md'
                      : isDark.value
                        ? 'text-gray-300 hover:text-blue-400 hover:bg-zinc-700'
                        : 'text-gray-600 hover:text-blue-600 hover:bg-blue-50'
                  }`}
                >
                  Home
                </button>
                <button
                  onClick$={() => navigate('/counter')}
                  class={`px-6 py-2 rounded-lg font-medium transition-all duration-200 ${
                    currentPath.value === '/counter'
                      ? 'bg-blue-600 text-white shadow-md'
                      : isDark.value
                        ? 'text-gray-300 hover:text-blue-400 hover:bg-zinc-700'
                        : 'text-gray-600 hover:text-blue-600 hover:bg-blue-50'
                  }`}
                >
                  Counter
                </button>
              </div>
              <button
                onClick$={toggleTheme}
                class={`p-2 rounded-lg transition-colors ${
                  isDark.value
                    ? 'bg-zinc-700 hover:bg-zinc-600 text-yellow-400'
                    : 'bg-gray-100 hover:bg-gray-200 text-gray-700'
                }`}
                title="Toggle theme"
              >
                {isDark.value ? (
                  <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"></path>
                  </svg>
                ) : (
                  <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"></path>
                  </svg>
                )}
              </button>
            </div>
          </div>
        </div>
      </nav>

      {/* Main Content */}
      <main class="max-w-6xl mx-auto px-4 py-12">
        {(currentPath.value === '/dashboard' || currentPath.value === '/') && (
          <div class="text-center">
            <div class="mb-8">
              <h1 class={`text-5xl font-bold mb-4 ${isDark.value ? 'text-gray-100' : 'text-gray-900'}`}>
                Welcome to Qwik SPA
              </h1>
              <p class={`text-xl max-w-2xl mx-auto leading-relaxed ${isDark.value ? 'text-gray-400' : 'text-gray-600'}`}>
                Experience lightning-fast interactivity with Qwik's resumable architecture.
                This single-page application loads instantly and stays responsive.
              </p>
            </div>

            {/* Middleware Demo Controls */}
            <div class={`mt-8 p-6 rounded-xl shadow-lg ${isDark.value ? 'bg-zinc-800' : 'bg-white'}`}>
              <h3 class={`text-lg font-semibold mb-4 ${isDark.value ? 'text-gray-100' : 'text-gray-900'}`}>
                Client-Side Middleware Demo
              </h3>
              <p class={`mb-4 ${isDark.value ? 'text-gray-400' : 'text-gray-600'}`}>
                Toggle permissions to test client-side middleware. Access checks happen entirely on the client without server requests.
              </p>

              <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
                <div class="flex items-center justify-between p-3 bg-opacity-50 rounded-lg">
                  <span class={`font-medium ${isDark.value ? 'text-gray-300' : 'text-gray-700'}`}>
                    Analytics Access
                  </span>
                  <button
                    onClick$={() => togglePermission('allowAnalytics')}
                    class={`px-4 py-2 rounded-lg font-medium transition-colors ${
                      middlewareConfig.value.allowAnalytics
                        ? 'bg-green-600 hover:bg-green-700 text-white'
                        : 'bg-red-600 hover:bg-red-700 text-white'
                    }`}
                  >
                    {middlewareConfig.value.allowAnalytics ? 'Allowed' : 'Denied'}
                  </button>
                </div>

                <div class="flex items-center justify-between p-3 bg-opacity-50 rounded-lg">
                  <span class={`font-medium ${isDark.value ? 'text-gray-300' : 'text-gray-700'}`}>
                    User Role
                  </span>
                  <button
                    onClick$={() => {
                      const roles: Array<'admin' | 'user' | 'guest'> = ['admin', 'user', 'guest']
                      const currentIndex = roles.indexOf(middlewareConfig.value.userRole)
                      const nextIndex = (currentIndex + 1) % roles.length
                      middlewareConfig.value = {
                        ...middlewareConfig.value,
                        userRole: roles[nextIndex]
                      }
                    }}
                    class={`px-4 py-2 rounded-lg font-medium transition-colors ${
                      isDark.value
                        ? 'bg-zinc-600 hover:bg-zinc-700 text-white'
                        : 'bg-gray-200 hover:bg-gray-300 text-gray-700'
                    }`}
                  >
                    {middlewareConfig.value.userRole}
                  </button>
                </div>
              </div>

              <p class={`mt-4 text-sm ${isDark.value ? 'text-gray-500' : 'text-gray-500'}`}>
                Try navigating to Analytics (needs permission) or Admin (needs admin role) to see middleware in action.
              </p>
            </div>

            <div class="grid md:grid-cols-3 gap-8 mt-12">
              <div class={`rounded-xl p-6 shadow-lg hover:shadow-xl transition-shadow duration-300 ${isDark.value ? 'bg-zinc-800' : 'bg-white'}`}>
                <div class="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center mb-4 mx-auto">
                  <svg class="w-6 h-6 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 10V3L4 14h7v7l9-11h-7z"></path>
                  </svg>
                </div>
                <h3 class={`text-lg font-semibold mb-2 ${isDark.value ? 'text-gray-100' : 'text-gray-900'}`}>Lightning Fast</h3>
                <p class={isDark.value ? 'text-gray-400' : 'text-gray-600'}>Zero hydration overhead with resumable components</p>
              </div>

              <div class={`rounded-xl p-6 shadow-lg hover:shadow-xl transition-shadow duration-300 ${isDark.value ? 'bg-zinc-800' : 'bg-white'}`}>
                <div class="w-12 h-12 bg-green-100 rounded-lg flex items-center justify-center mb-4 mx-auto">
                  <svg class="w-6 h-6 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"></path>
                  </svg>
                </div>
                <h3 class={`text-lg font-semibold mb-2 ${isDark.value ? 'text-gray-100' : 'text-gray-900'}`}>Client-Side Middleware</h3>
                <p class={isDark.value ? 'text-gray-400' : 'text-gray-600'}>Access control without server round trips</p>
              </div>

              <div class={`rounded-xl p-6 shadow-lg hover:shadow-xl transition-shadow duration-300 ${isDark.value ? 'bg-zinc-800' : 'bg-white'}`}>
                <div class="w-12 h-12 bg-purple-100 rounded-lg flex items-center justify-center mb-4 mx-auto">
                  <svg class="w-6 h-6 text-purple-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4.318 6.318a4.5 4.5 0 000 6.364L12 20.364l7.682-7.682a4.5 4.5 0 00-6.364-6.364L12 7.636l-1.318-1.318a4.5 4.5 0 00-6.364 0z"></path>
                  </svg>
                </div>
                <h3 class={`text-lg font-semibold mb-2 ${isDark.value ? 'text-gray-100' : 'text-gray-900'}`}>Developer Friendly</h3>
                <p class={isDark.value ? 'text-gray-400' : 'text-gray-600'}>Familiar React-like syntax with modern tooling</p>
              </div>
            </div>
          </div>
        )}

        {currentPath.value === '/counter' && (
          <div class="max-w-md mx-auto text-center">
            <div class={`p-8 rounded-xl shadow-lg ${isDark.value ? 'bg-zinc-800' : 'bg-white'}`}>
              <h1 class="text-3xl font-bold mb-8">Qwik Counter</h1>

              <div class="mb-8">
                <div class={`text-6xl font-bold mb-4 ${isDark.value ? 'text-blue-400' : 'text-blue-600'}`}>
                  {count.value}
                </div>
                <p class={isDark.value ? 'text-gray-400' : 'text-gray-600'}>
                  Current count value
                </p>
              </div>

              <div class="flex gap-4 justify-center">
                <button
                  onClick$={decrement}
                  class="px-6 py-3 bg-red-600 hover:bg-red-700 text-white font-semibold rounded-lg transition-colors"
                >
                  -1
                </button>

                <button
                  onClick$={reset}
                  class={`px-6 py-3 font-semibold rounded-lg transition-colors ${
                    isDark.value
                      ? 'bg-zinc-600 hover:bg-zinc-700 text-white'
                      : 'bg-gray-200 hover:bg-gray-300 text-gray-700'
                  }`}
                >
                  Reset
                </button>

                <button
                  onClick$={increment}
                  class="px-6 py-3 bg-green-600 hover:bg-green-700 text-white font-semibold rounded-lg transition-colors"
                >
                  +1
                </button>
              </div>

              <div class="mt-8 space-y-2">
                <div class={`text-sm ${isDark.value ? 'text-gray-400' : 'text-gray-600'}`}>
                  <strong>Framework:</strong> Qwik
                </div>
                <div class={`text-sm ${isDark.value ? 'text-gray-400' : 'text-gray-600'}`}>
                  <strong>State Management:</strong> useSignal()
                </div>
                <div class={`text-sm ${isDark.value ? 'text-gray-400' : 'text-gray-600'}`}>
                  <strong>Middleware:</strong> Client-side only
                </div>
              </div>
            </div>
          </div>
        )}
      </main>
    </div>
  )
})