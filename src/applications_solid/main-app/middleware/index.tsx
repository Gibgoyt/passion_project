import { createSignal, createContext, useContext, createEffect } from 'solid-js'
import type { Component, JSX } from 'solid-js'

interface MiddlewareContext {
  currentRoute: () => string
  beforeNavigate: (handler: (from: string, to: string) => void | boolean) => void
  afterNavigate: (handler: (to: string) => void) => void
  navigate: (path: string) => void
}

const MiddlewareContext = createContext<MiddlewareContext>()

export const useMiddleware = () => {
  const context = useContext(MiddlewareContext)
  if (!context) {
    throw new Error('useMiddleware must be used within MiddlewareProvider')
  }
  return context
}

interface MiddlewareProviderProps {
  children: JSX.Element
}

export const MiddlewareProvider: Component<MiddlewareProviderProps> = (props) => {
  const [currentRoute, setCurrentRoute] = createSignal(typeof window !== 'undefined' ? window.location.pathname || '/app/dashboard' : '/app/dashboard')
  const beforeHandlers: Array<(from: string, to: string) => void | boolean> = []
  const afterHandlers: Array<(to: string) => void> = []

  const beforeNavigate = (handler: (from: string, to: string) => void | boolean) => {
    beforeHandlers.push(handler)
  }

  const afterNavigate = (handler: (to: string) => void) => {
    afterHandlers.push(handler)
  }

  // Navigate programmatically
  const navigateTo = (path: string) => {
    const oldRoute = currentRoute()

    // Run before navigation handlers
    let shouldNavigate = true
    for (const handler of beforeHandlers) {
      const result = handler(oldRoute, path)
      if (result === false) {
        shouldNavigate = false
        break
      }
    }

    if (shouldNavigate) {
      window.history.pushState({}, '', path)
      setCurrentRoute(path)

      // Run after navigation handlers
      for (const handler of afterHandlers) {
        handler(path)
      }
    }
  }

  // Listen for popstate events (back/forward buttons)
  createEffect(() => {
    if (typeof window === 'undefined') return

    const handlePopState = () => {
      const newRoute = window.location.pathname
      const oldRoute = currentRoute()

      // Run before navigation handlers
      let shouldNavigate = true
      for (const handler of beforeHandlers) {
        const result = handler(oldRoute, newRoute)
        if (result === false) {
          shouldNavigate = false
          // Restore previous state if navigation prevented
          window.history.pushState({}, '', oldRoute)
          return
        }
      }

      if (shouldNavigate) {
        setCurrentRoute(newRoute)

        // Run after navigation handlers
        for (const handler of afterHandlers) {
          handler(newRoute)
        }
      }
    }

    window.addEventListener('popstate', handlePopState)

    return () => {
      window.removeEventListener('popstate', handlePopState)
    }
  })

  // Expose navigate function
  ;(window as any).__navigate = navigateTo

  const value: MiddlewareContext = {
    currentRoute,
    beforeNavigate,
    afterNavigate,
    navigate: navigateTo
  }

  return (
    <MiddlewareContext.Provider value={value}>
      {props.children}
    </MiddlewareContext.Provider>
  )
}

// Example middleware functions that can be used
export const authMiddleware = () => {
  // ALL PAGES OF THIS SOLID SPA **REQUIRE** AUTHENTICATION
  return (_from: string, to: string) => {
    // Hardcoded authentication check for testing - change this to false to test redirect
    const isAuthenticated = true// Change to false to test unauthenticated flow

    if (!isAuthenticated) {
      console.log('User not authenticated, redirecting to home')
      window.location.href = '/'
      return false
    }

    return true
  }
}

export const loggingMiddleware = () => {
  return (to: string) => {
    console.log(`Navigated to: ${to}`)
    // You could also send analytics here
  }
}