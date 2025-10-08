#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ======================
// Application State
// ======================
static int counter = 0;
static char current_route[256] = "/home";

// ======================
// JavaScript Interface
// ======================

// Get current hash from URL
EM_JS(void, js_get_hash, (char* buffer, int size), {
    const hash = window.location.hash || '#/home';
    // Remove the leading '#'
    const path = hash.substring(1);
    stringToUTF8(path, buffer, size);
});

// Set up hash change listener
EM_JS(void, js_setup_hash_listener, (), {
    window.addEventListener('hashchange', function() {
        Module._handle_route_change();
    });

    // Intercept clicks on navigation links
    document.addEventListener('click', function(e) {
        if (e.target.matches('a[data-route]')) {
            e.preventDefault();
            const route = e.target.getAttribute('data-route');
            window.location.hash = '#' + route;
        }
    });
});

// Update page content
EM_JS(void, js_set_content, (const char* html), {
    const root = document.getElementById('wasm-root');
    if (root) {
        root.innerHTML = UTF8ToString(html);
    }
});

// Update page title
EM_JS(void, js_set_title, (const char* title), {
    document.title = UTF8ToString(title);
});

// Update counter display
EM_JS(void, js_update_counter_display, (int value), {
    const counterEl = document.getElementById('counter-value');
    if (counterEl) {
        counterEl.textContent = value.toString();
    }
});

// ======================
// Navigation Component
// ======================

void render_navigation(const char* active_route) {
    char nav_html[2048];

    snprintf(nav_html, sizeof(nav_html),
        "<header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "  <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "    <div class=\"flex items-center justify-between h-16\">\n"
        "      <div class=\"flex items-center gap-3\">\n"
        "        <svg class=\"w-8 h-8 text-blue-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "          <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z\"></path>\n"
        "        </svg>\n"
        "        <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "      </div>\n"
        "      <nav class=\"flex gap-1\">\n"
        "        <a href=\"#/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors %s\">\n"
        "          Home\n"
        "        </a>\n"
        "        <a href=\"#/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors %s\">\n"
        "          About\n"
        "        </a>\n"
        "        <a href=\"#/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors %s\">\n"
        "          Counter\n"
        "        </a>\n"
        "        <a href=\"#/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors %s\">\n"
        "          Services\n"
        "        </a>\n"
        "      </nav>\n"
        "    </div>\n"
        "  </div>\n"
        "</header>\n",
        strcmp(active_route, "/home") == 0 || strcmp(active_route, "/") == 0 ?
            "bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300" :
            "text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700",
        strcmp(active_route, "/about") == 0 ?
            "bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300" :
            "text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700",
        strcmp(active_route, "/counter") == 0 ?
            "bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300" :
            "text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700",
        strcmp(active_route, "/services") == 0 ?
            "bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300" :
            "text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700"
    );

    // Store navigation HTML in a buffer for later use
    static char nav_buffer[2048];
    strncpy(nav_buffer, nav_html, sizeof(nav_buffer) - 1);
}

// ======================
// Page Rendering Functions
// ======================

void render_home() {
    js_set_title("Home - C/WASM SPA");

    render_navigation("/home");

    char html[8192];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-blue-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z\"></path>\n"
        "          </svg>\n"
        "          <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"#/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300\">Home</a>\n"
        "          <a href=\"#/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">About</a>\n"
        "          <a href=\"#/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Counter</a>\n"
        "          <a href=\"#/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-12\">\n"
        "    <div class=\"text-center mb-12\">\n"
        "      <h1 class=\"text-5xl font-bold text-gray-900 dark:text-white mb-4\">\n"
        "        Welcome to C/WASM SPA\n"
        "      </h1>\n"
        "      <p class=\"text-xl text-gray-600 dark:text-gray-300 max-w-2xl mx-auto\">\n"
        "        A multi-page Single Page Application built with pure C and compiled to WebAssembly using Emscripten\n"
        "      </p>\n"
        "    </div>\n"
        "    <div class=\"grid grid-cols-1 md:grid-cols-2 gap-6 max-w-4xl mx-auto\">\n"
        "      <a href=\"#/about\" data-route=\"/about\" class=\"block bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6 hover:shadow-md transition-shadow\">\n"
        "        <div class=\"flex items-center gap-3 mb-3\">\n"
        "          <svg class=\"w-6 h-6 text-blue-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z\"></path>\n"
        "          </svg>\n"
        "          <h3 class=\"text-lg font-semibold text-gray-900 dark:text-white\">About</h3>\n"
        "        </div>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Learn about the technology stack and architecture</p>\n"
        "      </a>\n"
        "      <a href=\"#/counter\" data-route=\"/counter\" class=\"block bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6 hover:shadow-md transition-shadow\">\n"
        "        <div class=\"flex items-center gap-3 mb-3\">\n"
        "          <svg class=\"w-6 h-6 text-green-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M7 11l5-5m0 0l5 5m-5-5v12\"></path>\n"
        "          </svg>\n"
        "          <h3 class=\"text-lg font-semibold text-gray-900 dark:text-white\">Interactive Counter</h3>\n"
        "        </div>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Try the WASM-powered counter demo</p>\n"
        "      </a>\n"
        "      <a href=\"#/services\" data-route=\"/services\" class=\"block bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6 hover:shadow-md transition-shadow\">\n"
        "        <div class=\"flex items-center gap-3 mb-3\">\n"
        "          <svg class=\"w-6 h-6 text-purple-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M4 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2V6zM14 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2V6zM4 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2v-2zM14 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2v-2z\"></path>\n"
        "          </svg>\n"
        "          <h3 class=\"text-lg font-semibold text-gray-900 dark:text-white\">Services</h3>\n"
        "        </div>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Explore what this SPA can do</p>\n"
        "      </a>\n"
        "      <div class=\"bg-gradient-to-br from-blue-50 to-purple-50 dark:from-blue-900/20 dark:to-purple-900/20 rounded-lg shadow-sm border border-blue-200 dark:border-blue-800 p-6\">\n"
        "        <div class=\"flex items-center gap-3 mb-3\">\n"
        "          <svg class=\"w-6 h-6 text-yellow-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "          <h3 class=\"text-lg font-semibold text-gray-900 dark:text-white\">Current Counter</h3>\n"
        "        </div>\n"
        "        <p class=\"text-3xl font-bold text-blue-600 dark:text-blue-400\">%d</p>\n"
        "        <p class=\"text-sm text-gray-600 dark:text-gray-400 mt-2\">State persists across navigation</p>\n"
        "      </div>\n"
        "    </div>\n"
        "  </main>\n"
        "</div>",
        counter
    );

    js_set_content(html);
    printf("📄 Rendered: Home page (counter: %d)\n", counter);
}

void render_about() {
    js_set_title("About - C/WASM SPA");

    char html[8192];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-blue-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z\"></path>\n"
        "          </svg>\n"
        "          <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"#/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Home</a>\n"
        "          <a href=\"#/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300\">About</a>\n"
        "          <a href=\"#/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Counter</a>\n"
        "          <a href=\"#/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-4xl mx-auto px-4 sm:px-6 lg:px-8 py-12\">\n"
        "    <h1 class=\"text-4xl font-bold text-gray-900 dark:text-white mb-6\">About This Application</h1>\n"
        "    <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-8 mb-6\">\n"
        "      <h2 class=\"text-2xl font-semibold text-gray-900 dark:text-white mb-4\">Technology Stack</h2>\n"
        "      <div class=\"grid grid-cols-1 md:grid-cols-3 gap-4\">\n"
        "        <div class=\"bg-blue-50 dark:bg-blue-900/20 rounded-lg p-4 border border-blue-200 dark:border-blue-800\">\n"
        "          <div class=\"text-blue-600 dark:text-blue-400 text-sm font-medium mb-1\">Language</div>\n"
        "          <div class=\"text-2xl font-bold text-blue-900 dark:text-blue-100\">C</div>\n"
        "        </div>\n"
        "        <div class=\"bg-purple-50 dark:bg-purple-900/20 rounded-lg p-4 border border-purple-200 dark:border-purple-800\">\n"
        "          <div class=\"text-purple-600 dark:text-purple-400 text-sm font-medium mb-1\">Compiler</div>\n"
        "          <div class=\"text-2xl font-bold text-purple-900 dark:text-purple-100\">Emscripten</div>\n"
        "        </div>\n"
        "        <div class=\"bg-green-50 dark:bg-green-900/20 rounded-lg p-4 border border-green-200 dark:border-green-800\">\n"
        "          <div class=\"text-green-600 dark:text-green-400 text-sm font-medium mb-1\">Runtime</div>\n"
        "          <div class=\"text-2xl font-bold text-green-900 dark:text-green-100\">WebAssembly</div>\n"
        "        </div>\n"
        "      </div>\n"
        "    </div>\n"
        "    <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-8 mb-6\">\n"
        "      <h2 class=\"text-2xl font-semibold text-gray-900 dark:text-white mb-4\">Features</h2>\n"
        "      <ul class=\"space-y-3\">\n"
        "        <li class=\"flex items-start gap-3\">\n"
        "          <svg class=\"w-6 h-6 text-green-500 flex-shrink-0 mt-0.5\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M5 13l4 4L19 7\"></path>\n"
        "          </svg>\n"
        "          <span class=\"text-gray-700 dark:text-gray-300\"><strong>Hash-based routing</strong> - Client-side navigation without page refreshes</span>\n"
        "        </li>\n"
        "        <li class=\"flex items-start gap-3\">\n"
        "          <svg class=\"w-6 h-6 text-green-500 flex-shrink-0 mt-0.5\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M5 13l4 4L19 7\"></path>\n"
        "          </svg>\n"
        "          <span class=\"text-gray-700 dark:text-gray-300\"><strong>Pure C implementation</strong> - All logic written in C, compiled to WASM</span>\n"
        "        </li>\n"
        "        <li class=\"flex items-start gap-3\">\n"
        "          <svg class=\"w-6 h-6 text-green-500 flex-shrink-0 mt-0.5\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M5 13l4 4L19 7\"></path>\n"
        "          </svg>\n"
        "          <span class=\"text-gray-700 dark:text-gray-300\"><strong>State management</strong> - Counter state persists across page navigation</span>\n"
        "        </li>\n"
        "        <li class=\"flex items-start gap-3\">\n"
        "          <svg class=\"w-6 h-6 text-green-500 flex-shrink-0 mt-0.5\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M5 13l4 4L19 7\"></path>\n"
        "          </svg>\n"
        "          <span class=\"text-gray-700 dark:text-gray-300\"><strong>Browser integration</strong> - Back/forward button support via hashchange events</span>\n"
        "        </li>\n"
        "        <li class=\"flex items-start gap-3\">\n"
        "          <svg class=\"w-6 h-6 text-green-500 flex-shrink-0 mt-0.5\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M5 13l4 4L19 7\"></path>\n"
        "          </svg>\n"
        "          <span class=\"text-gray-700 dark:text-gray-300\"><strong>Responsive design</strong> - Tailwind CSS with dark mode support</span>\n"
        "        </li>\n"
        "      </ul>\n"
        "    </div>\n"
        "    <div class=\"bg-gradient-to-br from-blue-50 to-purple-50 dark:from-blue-900/20 dark:to-purple-900/20 rounded-lg border border-blue-200 dark:border-blue-800 p-8\">\n"
        "      <h2 class=\"text-2xl font-semibold text-gray-900 dark:text-white mb-4\">How It Works</h2>\n"
        "      <p class=\"text-gray-700 dark:text-gray-300 mb-4\">\n"
        "        This application uses Emscripten's <code class=\"bg-gray-200 dark:bg-zinc-700 px-2 py-1 rounded\">EM_JS</code> macro to create JavaScript functions that can be called from C code. The router listens for hash changes and calls the appropriate C rendering function.\n"
        "      </p>\n"
        "      <p class=\"text-gray-700 dark:text-gray-300\">\n"
        "        When you navigate between pages, the C code generates HTML strings and passes them to JavaScript to update the DOM - all without reloading the page!\n"
        "      </p>\n"
        "    </div>\n"
        "  </main>\n"
        "</div>"
    );

    js_set_content(html);
    printf("📄 Rendered: About page\n");
}

void render_counter() {
    js_set_title("Counter - C/WASM SPA");

    char html[8192];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-blue-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z\"></path>\n"
        "          </svg>\n"
        "          <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"#/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Home</a>\n"
        "          <a href=\"#/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">About</a>\n"
        "          <a href=\"#/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300\">Counter</a>\n"
        "          <a href=\"#/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-4xl mx-auto px-4 sm:px-6 lg:px-8 py-12\">\n"
        "    <h1 class=\"text-4xl font-bold text-gray-900 dark:text-white mb-8 text-center\">Interactive Counter</h1>\n"
        "    <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-8\">\n"
        "      <div class=\"flex flex-col items-center gap-8\">\n"
        "        <div class=\"text-center\">\n"
        "          <div class=\"text-8xl font-bold text-blue-600 dark:text-blue-400\" id=\"counter-value\">%d</div>\n"
        "          <div class=\"text-sm text-gray-500 dark:text-gray-400 mt-4\">Current Count</div>\n"
        "        </div>\n"
        "        <div class=\"flex gap-4\">\n"
        "          <button onclick=\"Module._decrement_counter()\" class=\"px-8 py-4 bg-red-500 hover:bg-red-600 text-white font-medium rounded-lg transition-colors shadow-lg hover:shadow-xl transform hover:scale-105 active:scale-95\">\n"
        "            <svg class=\"w-6 h-6\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "              <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M20 12H4\"></path>\n"
        "            </svg>\n"
        "          </button>\n"
        "          <button onclick=\"Module._reset_counter()\" class=\"px-8 py-4 bg-gray-500 hover:bg-gray-600 text-white font-medium rounded-lg transition-colors shadow-lg hover:shadow-xl transform hover:scale-105 active:scale-95\">\n"
        "            Reset\n"
        "          </button>\n"
        "          <button onclick=\"Module._increment_counter()\" class=\"px-8 py-4 bg-green-500 hover:bg-green-600 text-white font-medium rounded-lg transition-colors shadow-lg hover:shadow-xl transform hover:scale-105 active:scale-95\">\n"
        "            <svg class=\"w-6 h-6\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "              <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M12 4v16m8-8H4\"></path>\n"
        "            </svg>\n"
        "          </button>\n"
        "        </div>\n"
        "        <div class=\"text-center max-w-md\">\n"
        "          <p class=\"text-gray-600 dark:text-gray-300\">\n"
        "            This counter is managed entirely by C code running in WebAssembly. Try navigating to other pages and coming back - the counter value persists!\n"
        "          </p>\n"
        "        </div>\n"
        "      </div>\n"
        "    </div>\n"
        "    <div class=\"mt-8 bg-blue-50 dark:bg-blue-900/20 rounded-lg border border-blue-200 dark:border-blue-800 p-6\">\n"
        "      <h3 class=\"text-lg font-semibold text-blue-900 dark:text-blue-100 mb-2\">💡 Try This</h3>\n"
        "      <ul class=\"space-y-2 text-blue-800 dark:text-blue-200\">\n"
        "        <li>• Click increment/decrement to change the counter</li>\n"
        "        <li>• Navigate to another page</li>\n"
        "        <li>• Come back to see the counter value is preserved</li>\n"
        "        <li>• Check the home page - it also shows the current counter!</li>\n"
        "      </ul>\n"
        "    </div>\n"
        "  </main>\n"
        "</div>",
        counter
    );

    js_set_content(html);
    printf("📄 Rendered: Counter page (value: %d)\n", counter);
}

void render_services() {
    js_set_title("Services - C/WASM SPA");

    char html[8192];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-blue-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z\"></path>\n"
        "          </svg>\n"
        "          <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"#/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Home</a>\n"
        "          <a href=\"#/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">About</a>\n"
        "          <a href=\"#/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Counter</a>\n"
        "          <a href=\"#/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors bg-blue-100 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-6xl mx-auto px-4 sm:px-6 lg:px-8 py-12\">\n"
        "    <h1 class=\"text-4xl font-bold text-gray-900 dark:text-white mb-4 text-center\">What This SPA Offers</h1>\n"
        "    <p class=\"text-xl text-gray-600 dark:text-gray-300 text-center mb-12 max-w-3xl mx-auto\">\n"
        "      Explore the capabilities of WebAssembly-powered applications\n"
        "    </p>\n"
        "    <div class=\"grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6\">\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-blue-100 dark:bg-blue-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-blue-600 dark:text-blue-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">High Performance</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">WebAssembly runs at near-native speed, making it perfect for computationally intensive tasks</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-purple-100 dark:bg-purple-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-purple-600 dark:text-purple-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M10 20l4-16m4 4l4 4-4 4M6 16l-4-4 4-4\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">Client-Side Routing</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Fast navigation between pages without server round-trips or page reloads</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-green-100 dark:bg-green-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-green-600 dark:text-green-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9 12l2 2 4-4m5.618-4.016A11.955 11.955 0 0112 2.944a11.955 11.955 0 01-8.618 3.04A12.02 12.02 0 003 9c0 5.591 3.824 10.29 9 11.622 5.176-1.332 9-6.03 9-11.622 0-1.042-.133-2.052-.382-3.016z\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">Memory Safety</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">C code compiled to WASM runs in a sandboxed environment, ensuring security</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-yellow-100 dark:bg-yellow-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-yellow-600 dark:text-yellow-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M4 7v10c0 2.21 3.582 4 8 4s8-1.79 8-4V7M4 7c0 2.21 3.582 4 8 4s8-1.79 8-4M4 7c0-2.21 3.582-4 8-4s8 1.79 8 4\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">State Management</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Application state persists across page navigation, just like modern SPAs</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-red-100 dark:bg-red-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-red-600 dark:text-red-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M12 18h.01M8 21h8a2 2 0 002-2V5a2 2 0 00-2-2H8a2 2 0 00-2 2v14a2 2 0 002 2z\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">Responsive Design</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Mobile-friendly interface with Tailwind CSS and dark mode support</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-indigo-100 dark:bg-indigo-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-indigo-600 dark:text-indigo-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M7 21a4 4 0 01-4-4V5a2 2 0 012-2h4a2 2 0 012 2v12a4 4 0 01-4 4zm0 0h12a2 2 0 002-2v-4a2 2 0 00-2-2h-2.343M11 7.343l1.657-1.657a2 2 0 012.828 0l2.829 2.829a2 2 0 010 2.828l-8.486 8.485M7 17h.01\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">Extensible</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Easy to add new pages and features by extending the C router</p>\n"
        "      </div>\n"
        "    </div>\n"
        "    <div class=\"mt-12 bg-gradient-to-r from-blue-500 to-purple-600 rounded-lg p-8 text-center text-white\">\n"
        "      <h2 class=\"text-3xl font-bold mb-4\">Ready to Build?</h2>\n"
        "      <p class=\"text-lg mb-6 opacity-90\">\n"
        "        Check out the <a href=\"#/about\" data-route=\"/about\" class=\"underline font-semibold\">About page</a> to learn more about the architecture\n"
        "      </p>\n"
        "      <a href=\"#/counter\" data-route=\"/counter\" class=\"inline-block bg-white text-blue-600 px-8 py-3 rounded-lg font-semibold hover:bg-gray-100 transition-colors\">\n"
        "        Try the Counter Demo\n"
        "      </a>\n"
        "    </div>\n"
        "  </main>\n"
        "</div>"
    );

    js_set_content(html);
    printf("📄 Rendered: Services page\n");
}

void render_404() {
    js_set_title("404 - Page Not Found");

    char html[4096];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-blue-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z\"></path>\n"
        "          </svg>\n"
        "          <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"#/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Home</a>\n"
        "          <a href=\"#/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">About</a>\n"
        "          <a href=\"#/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Counter</a>\n"
        "          <a href=\"#/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-4xl mx-auto px-4 sm:px-6 lg:px-8 py-12 text-center\">\n"
        "    <div class=\"mb-8\">\n"
        "      <svg class=\"w-32 h-32 mx-auto text-gray-400 dark:text-gray-600\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "        <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9.172 16.172a4 4 0 015.656 0M9 10h.01M15 10h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z\"></path>\n"
        "      </svg>\n"
        "    </div>\n"
        "    <h1 class=\"text-6xl font-bold text-gray-900 dark:text-white mb-4\">404</h1>\n"
        "    <h2 class=\"text-2xl font-semibold text-gray-700 dark:text-gray-300 mb-4\">Page Not Found</h2>\n"
        "    <p class=\"text-gray-600 dark:text-gray-400 mb-8\">\n"
        "      The page you're looking for doesn't exist in this WebAssembly SPA.\n"
        "    </p>\n"
        "    <a href=\"#/home\" data-route=\"/home\" class=\"inline-block px-6 py-3 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors\">\n"
        "      Return to Home\n"
        "    </a>\n"
        "  </main>\n"
        "</div>"
    );

    js_set_content(html);
    printf("📄 Rendered: 404 page\n");
}

// ======================
// Counter Functions (Exported for JS)
// ======================

EMSCRIPTEN_KEEPALIVE
void increment_counter() {
    counter++;
    js_update_counter_display(counter);
    printf("✅ Counter incremented to: %d\n", counter);
}

EMSCRIPTEN_KEEPALIVE
void decrement_counter() {
    counter--;
    js_update_counter_display(counter);
    printf("✅ Counter decremented to: %d\n", counter);
}

EMSCRIPTEN_KEEPALIVE
void reset_counter() {
    counter = 0;
    js_update_counter_display(counter);
    printf("✅ Counter reset to: 0\n");
}

// ======================
// Router
// ======================

EMSCRIPTEN_KEEPALIVE
void handle_route_change() {
    js_get_hash(current_route, sizeof(current_route));

    printf("🔀 Navigating to: %s\n", current_route);

    // Route matching
    if (strcmp(current_route, "/") == 0 || strcmp(current_route, "/home") == 0 || strcmp(current_route, "") == 0) {
        render_home();
    } else if (strcmp(current_route, "/about") == 0) {
        render_about();
    } else if (strcmp(current_route, "/counter") == 0) {
        render_counter();
    } else if (strcmp(current_route, "/services") == 0) {
        render_services();
    } else {
        render_404();
    }
}

EMSCRIPTEN_KEEPALIVE
void navigate_to(const char* path) {
    EM_ASM({
        window.location.hash = '#' + UTF8ToString($0);
    }, path);
}

// ======================
// Application Entry Point
// ======================

int main() {
    printf("🚀 C/WASM Multi-Page SPA Starting...\n");
    printf("📦 Setting up hash-based routing system\n");

    // Set up hash change listener
    js_setup_hash_listener();

    // Route to initial page
    handle_route_change();

    printf("✅ Application initialized successfully!\n");
    printf("🌐 Available routes: /home, /about, /counter, /services\n");

    return 0;
}
