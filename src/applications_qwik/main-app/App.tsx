/** @jsxImportSource @builder.io/qwik */
import { component$, useSignal, $, useVisibleTask$, useStore } from '@builder.io/qwik';

export const App = component$(() => {
  const currentPath = useSignal('/');
  const isDark = useSignal(false);
  const count = useSignal(0);

  const navigate = $((path: string) => {
    currentPath.value = path;
    // Update URL without page reload
    if (typeof window !== 'undefined') {
      const fullPath = `/qwik-spa${path}`;
      window.history.pushState({}, '', fullPath);
    }
  });

  // Initialize dark mode from localStorage/Astro app
  useVisibleTask$(() => {
    const isDarkMode = localStorage.getItem('darkMode') === 'true' ||
      (!localStorage.getItem('darkMode') && window.matchMedia('(prefers-color-scheme: dark)').matches) ||
      document.documentElement.classList.contains('dark');

    isDark.value = isDarkMode;
    updateTheme(isDarkMode);
  });

  const updateTheme = $((dark: boolean) => {
    isDark.value = dark;
    localStorage.setItem('darkMode', dark.toString());
    if (dark) {
      document.documentElement.classList.add('dark');
    } else {
      document.documentElement.classList.remove('dark');
    }
  });

  const toggleTheme = $(() => {
    updateTheme(!isDark.value);
  });

  const increment = $(() => {
    count.value++;
  });

  const decrement = $(() => {
    count.value--;
  });

  const reset = $(() => {
    count.value = 0;
  });

  return (
    <div class={`min-h-screen transition-colors ${isDark.value ? 'bg-zinc-900 text-gray-100' : 'bg-gradient-to-br from-blue-50 to-indigo-100'}`}>
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
                  onClick$={() => navigate('/')}
                  class={`px-6 py-2 rounded-lg font-medium transition-all duration-200 ${
                    currentPath.value === '/'
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
                <button
                  onClick$={() => navigate('/about')}
                  class={`px-6 py-2 rounded-lg font-medium transition-all duration-200 ${
                    currentPath.value === '/about'
                      ? 'bg-blue-600 text-white shadow-md'
                      : isDark.value
                        ? 'text-gray-300 hover:text-blue-400 hover:bg-zinc-700'
                        : 'text-gray-600 hover:text-blue-600 hover:bg-blue-50'
                  }`}
                >
                  About
                </button>
                <button
                  onClick$={() => navigate('/contact')}
                  class={`px-6 py-2 rounded-lg font-medium transition-all duration-200 ${
                    currentPath.value === '/contact'
                      ? 'bg-blue-600 text-white shadow-md'
                      : isDark.value
                        ? 'text-gray-300 hover:text-blue-400 hover:bg-zinc-700'
                        : 'text-gray-600 hover:text-blue-600 hover:bg-blue-50'
                  }`}
                >
                  Contact
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
        {currentPath.value === '/' && (
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
                <h3 class={`text-lg font-semibold mb-2 ${isDark.value ? 'text-gray-100' : 'text-gray-900'}`}>SEO Friendly</h3>
                <p class={isDark.value ? 'text-gray-400' : 'text-gray-600'}>Server-side rendering with client-side interactivity</p>
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
                  <strong>Reactivity:</strong> Fine-grained updates
                </div>
              </div>
            </div>
          </div>
        )}

        {currentPath.value === '/about' && (
          <div class="max-w-4xl mx-auto">
            <div class="text-center mb-12">
              <h1 class="text-4xl font-bold text-gray-900 mb-4">About This Project</h1>
              <p class="text-xl text-gray-600">Learn more about our Qwik-powered application</p>
            </div>

            <div class="grid md:grid-cols-2 gap-12 items-start">
              <div class="space-y-6">
                <div class="bg-white rounded-xl p-8 shadow-lg">
                  <h2 class="text-2xl font-semibold text-gray-900 mb-4">What is Qwik?</h2>
                  <p class="text-gray-600 leading-relaxed mb-4">
                    Qwik is a new kind of web framework that can deliver instant loading web applications
                    at any size or complexity. Qwik achieves this through two main strategies:
                  </p>
                  <ul class="space-y-2 text-gray-600">
                    <li class="flex items-start">
                      <span class="w-2 h-2 bg-blue-500 rounded-full mt-2 mr-3 flex-shrink-0"></span>
                      Delay execution and download of JavaScript for as long as possible
                    </li>
                    <li class="flex items-start">
                      <span class="w-2 h-2 bg-blue-500 rounded-full mt-2 mr-3 flex-shrink-0"></span>
                      Use resumability instead of hydration for instant-on interactivity
                    </li>
                  </ul>
                </div>

                <div class="bg-white rounded-xl p-8 shadow-lg">
                  <h2 class="text-2xl font-semibold text-gray-900 mb-4">Performance Benefits</h2>
                  <div class="space-y-4">
                    <div class="flex items-center justify-between p-3 bg-green-50 rounded-lg">
                      <span class="text-gray-700">Time to Interactive</span>
                      <span class="font-semibold text-green-600">~0ms</span>
                    </div>
                    <div class="flex items-center justify-between p-3 bg-blue-50 rounded-lg">
                      <span class="text-gray-700">Initial JavaScript</span>
                      <span class="font-semibold text-blue-600">~1KB</span>
                    </div>
                    <div class="flex items-center justify-between p-3 bg-purple-50 rounded-lg">
                      <span class="text-gray-700">First Load</span>
                      <span class="font-semibold text-purple-600">Instant</span>
                    </div>
                  </div>
                </div>
              </div>

              <div class="space-y-6">
                <div class="bg-gradient-to-r from-blue-600 to-purple-600 rounded-xl p-8 text-white">
                  <h2 class="text-2xl font-semibold mb-4">Why Choose Qwik?</h2>
                  <div class="space-y-4">
                    <div class="flex items-start">
                      <div class="w-8 h-8 bg-white bg-opacity-20 rounded-lg flex items-center justify-center mr-4 mt-1">
                        <span class="text-sm font-bold">1</span>
                      </div>
                      <div>
                        <h3 class="font-medium mb-1">Instant Loading</h3>
                        <p class="text-blue-100 text-sm">No matter how complex your app, it loads instantly</p>
                      </div>
                    </div>

                    <div class="flex items-start">
                      <div class="w-8 h-8 bg-white bg-opacity-20 rounded-lg flex items-center justify-center mr-4 mt-1">
                        <span class="text-sm font-bold">2</span>
                      </div>
                      <div>
                        <h3 class="font-medium mb-1">Progressive Enhancement</h3>
                        <p class="text-blue-100 text-sm">JavaScript loads only when and where needed</p>
                      </div>
                    </div>

                    <div class="flex items-start">
                      <div class="w-8 h-8 bg-white bg-opacity-20 rounded-lg flex items-center justify-center mr-4 mt-1">
                        <span class="text-sm font-bold">3</span>
                      </div>
                      <div>
                        <h3 class="font-medium mb-1">Familiar Syntax</h3>
                        <p class="text-blue-100 text-sm">React-like components with modern features</p>
                      </div>
                    </div>
                  </div>
                </div>

                <div class="bg-white rounded-xl p-8 shadow-lg">
                  <h2 class="text-2xl font-semibold text-gray-900 mb-4">Built With</h2>
                  <div class="flex flex-wrap gap-3">
                    <span class="px-3 py-1 bg-blue-100 text-blue-800 text-sm font-medium rounded-full">Qwik</span>
                    <span class="px-3 py-1 bg-gray-100 text-gray-800 text-sm font-medium rounded-full">Astro</span>
                    <span class="px-3 py-1 bg-cyan-100 text-cyan-800 text-sm font-medium rounded-full">Tailwind CSS</span>
                    <span class="px-3 py-1 bg-blue-100 text-blue-800 text-sm font-medium rounded-full">TypeScript</span>
                  </div>
                </div>
              </div>
            </div>
          </div>
        )}

        {currentPath.value === '/contact' && (
          <div class="max-w-3xl mx-auto">
            <div class="text-center mb-12">
              <h1 class="text-4xl font-bold text-gray-900 mb-4">Get In Touch</h1>
              <p class="text-xl text-gray-600">We'd love to hear from you. Send us a message!</p>
            </div>

            <div class="grid md:grid-cols-2 gap-12">
              <div class="space-y-8">
                <div class="bg-white rounded-xl p-8 shadow-lg">
                  <h2 class="text-2xl font-semibold text-gray-900 mb-6">Contact Information</h2>

                  <div class="space-y-4">
                    <div class="flex items-center">
                      <div class="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center mr-4">
                        <svg class="w-6 h-6 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 8l7.89 4.26a2 2 0 002.22 0L21 8M5 19h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v10a2 2 0 002 2z"></path>
                        </svg>
                      </div>
                      <div>
                        <h3 class="font-medium text-gray-900">Email</h3>
                        <p class="text-gray-600">hello@qwikspa.dev</p>
                      </div>
                    </div>

                    <div class="flex items-center">
                      <div class="w-12 h-12 bg-green-100 rounded-lg flex items-center justify-center mr-4">
                        <svg class="w-6 h-6 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 5a2 2 0 012-2h3.28a1 1 0 01.948.684l1.498 4.493a1 1 0 01-.502 1.21l-2.257 1.13a11.042 11.042 0 005.516 5.516l1.13-2.257a1 1 0 011.21-.502l4.493 1.498a1 1 0 01.684.949V19a2 2 0 01-2 2h-1C9.716 21 3 14.284 3 6V5z"></path>
                        </svg>
                      </div>
                      <div>
                        <h3 class="font-medium text-gray-900">Phone</h3>
                        <p class="text-gray-600">+1 (555) 123-4567</p>
                      </div>
                    </div>

                    <div class="flex items-center">
                      <div class="w-12 h-12 bg-purple-100 rounded-lg flex items-center justify-center mr-4">
                        <svg class="w-6 h-6 text-purple-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M17.657 16.657L13.414 20.9a1.998 1.998 0 01-2.827 0l-4.244-4.243a8 8 0 1111.314 0z"></path>
                          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 11a3 3 0 11-6 0 3 3 0 016 0z"></path>
                        </svg>
                      </div>
                      <div>
                        <h3 class="font-medium text-gray-900">Location</h3>
                        <p class="text-gray-600">San Francisco, CA</p>
                      </div>
                    </div>
                  </div>
                </div>

                <div class="bg-gradient-to-r from-blue-600 to-purple-600 rounded-xl p-8 text-white">
                  <h2 class="text-xl font-semibold mb-4">Follow Us</h2>
                  <div class="flex space-x-4">
                    <a href="#" class="w-10 h-10 bg-white bg-opacity-20 rounded-lg flex items-center justify-center hover:bg-opacity-30 transition-colors">
                      <span class="sr-only">Twitter</span>
                      <svg class="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
                        <path d="M6.29 18.251c7.547 0 11.675-6.253 11.675-11.675 0-.178 0-.355-.012-.53A8.348 8.348 0 0020 3.92a8.19 8.19 0 01-2.357.646 4.118 4.118 0 001.804-2.27 8.224 8.224 0 01-2.605.996 4.107 4.107 0 00-6.993 3.743 11.65 11.65 0 01-8.457-4.287 4.106 4.106 0 001.27 5.477A4.073 4.073 0 01.8 7.713v.052a4.105 4.105 0 003.292 4.022 4.095 4.095 0 01-1.853.07 4.108 4.108 0 003.834 2.85A8.233 8.233 0 010 16.407a11.616 11.616 0 006.29 1.84"></path>
                      </svg>
                    </a>
                    <a href="#" class="w-10 h-10 bg-white bg-opacity-20 rounded-lg flex items-center justify-center hover:bg-opacity-30 transition-colors">
                      <span class="sr-only">GitHub</span>
                      <svg class="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
                        <path fill-rule="evenodd" d="M10 0C4.477 0 0 4.484 0 10.017c0 4.425 2.865 8.18 6.839 9.504.5.092.682-.217.682-.483 0-.237-.008-.868-.013-1.703-2.782.605-3.369-1.343-3.369-1.343-.454-1.158-1.11-1.466-1.11-1.466-.908-.62.069-.608.069-.608 1.003.07 1.531 1.032 1.531 1.032.892 1.53 2.341 1.088 2.91.832.092-.647.35-1.088.636-1.338-2.22-.253-4.555-1.113-4.555-4.951 0-1.093.39-1.988 1.029-2.688-.103-.253-.446-1.272.098-2.65 0 0 .84-.27 2.75 1.026A9.564 9.564 0 0110 4.844c.85.004 1.705.115 2.504.337 1.909-1.296 2.747-1.027 2.747-1.027.546 1.379.203 2.398.1 2.651.64.7 1.028 1.595 1.028 2.688 0 3.848-2.339 4.695-4.566 4.942.359.31.678.921.678 1.856 0 1.338-.012 2.419-.012 2.747 0 .268.18.58.688.482A10.019 10.019 0 0020 10.017C20 4.484 15.522 0 10 0z" clip-rule="evenodd"></path>
                      </svg>
                    </a>
                    <a href="#" class="w-10 h-10 bg-white bg-opacity-20 rounded-lg flex items-center justify-center hover:bg-opacity-30 transition-colors">
                      <span class="sr-only">LinkedIn</span>
                      <svg class="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
                        <path fill-rule="evenodd" d="M16.338 16.338H13.67V12.16c0-.995-.017-2.277-1.387-2.277-1.39 0-1.601 1.086-1.601 2.207v4.248H8.014v-8.59h2.559v1.174h.037c.356-.675 1.227-1.387 2.526-1.387 2.703 0 3.203 1.778 3.203 4.092v4.711zM5.005 6.575a1.548 1.548 0 11-.003-3.096 1.548 1.548 0 01.003 3.096zm-1.337 9.763H6.34v-8.59H3.667v8.59zM17.668 1H2.328C1.595 1 1 1.581 1 2.298v15.403C1 18.418 1.595 19 2.328 19h15.34c.734 0 1.332-.582 1.332-1.299V2.298C19 1.581 18.402 1 17.668 1z" clip-rule="evenodd"></path>
                      </svg>
                    </a>
                  </div>
                </div>
              </div>

              <div class="bg-white rounded-xl shadow-lg p-8">
                <h2 class="text-2xl font-semibold text-gray-900 mb-6">Send a Message</h2>

                <form class="space-y-6">
                  <div>
                    <label for="name" class="block text-sm font-medium text-gray-700 mb-2">
                      Full Name
                    </label>
                    <input
                      type="text"
                      id="name"
                      class="w-full px-4 py-3 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-colors"
                      placeholder="Your full name"
                    />
                  </div>

                  <div>
                    <label for="email" class="block text-sm font-medium text-gray-700 mb-2">
                      Email Address
                    </label>
                    <input
                      type="email"
                      id="email"
                      class="w-full px-4 py-3 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-colors"
                      placeholder="your.email@example.com"
                    />
                  </div>

                  <div>
                    <label for="subject" class="block text-sm font-medium text-gray-700 mb-2">
                      Subject
                    </label>
                    <input
                      type="text"
                      id="subject"
                      class="w-full px-4 py-3 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-colors"
                      placeholder="What's this about?"
                    />
                  </div>

                  <div>
                    <label for="message" class="block text-sm font-medium text-gray-700 mb-2">
                      Message
                    </label>
                    <textarea
                      id="message"
                      rows={5}
                      class="w-full px-4 py-3 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-colors resize-none"
                      placeholder="Tell us more about your message..."
                    ></textarea>
                  </div>

                  <button
                    type="submit"
                    class="w-full bg-gradient-to-r from-blue-600 to-purple-600 text-white font-medium py-3 px-6 rounded-lg hover:from-blue-700 hover:to-purple-700 transform hover:scale-105 transition-all duration-200 shadow-lg"
                  >
                    Send Message
                  </button>
                </form>
              </div>
            </div>
          </div>
        )}
      </main>
    </div>
  );
});