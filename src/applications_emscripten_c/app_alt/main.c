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
// JavaScript Interface - History API
// ======================

// Get current pathname from URL
EM_JS(void, js_get_pathname, (char* buffer, int size), {
    let path = window.location.pathname || '/';
    // Extract route after the base path (/emscripten-c-spa-alt)
    const basePath = '/emscripten-c-spa-alt';
    if (path.startsWith(basePath)) {
        path = path.substring(basePath.length);
    }
    if (!path || path === '/') {
        path = '/home';
    }
    stringToUTF8(path, buffer, size);
});

// Push state to history (navigate programmatically)
EM_JS(void, js_push_state, (const char* path), {
    const pathStr = UTF8ToString(path);
    const fullPath = '/emscripten-c-spa-alt' + pathStr;
    history.pushState({}, '', fullPath);
    Module._handle_route_change();
});

// Set up popstate listener for browser back/forward
EM_JS(void, js_setup_popstate_listener, (), {
    window.addEventListener('popstate', function(event) {
        Module._handle_route_change();
    });

    // Intercept clicks on navigation links
    document.addEventListener('click', function(e) {
        if (e.target.matches('a[data-route]')) {
            e.preventDefault();
            const route = e.target.getAttribute('data-route');
            const fullPath = '/emscripten-c-spa-alt' + route;
            history.pushState({}, '', fullPath);
            Module._handle_route_change();
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
// Page Rendering Functions
// ======================

void render_home() {
    js_set_title("Home - C/WASM SPA (History API)");

    char html[8192];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-purple-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "          <div>\n"
        "            <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "            <p class=\"text-xs text-purple-600 dark:text-purple-400\">History API Mode</p>\n"
        "          </div>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"/emscripten-c-spa-alt/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors bg-purple-100 dark:bg-purple-900/30 text-purple-700 dark:text-purple-300\">Home</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">About</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Counter</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-12\">\n"
        "    <div class=\"bg-purple-50 dark:bg-purple-900/20 border border-purple-200 dark:border-purple-800 rounded-lg p-4 mb-8\">\n"
        "      <div class=\"flex items-center gap-3\">\n"
        "        <svg class=\"w-5 h-5 text-purple-600 dark:text-purple-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "          <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z\"></path>\n"
        "        </svg>\n"
        "        <p class=\"text-sm text-purple-900 dark:text-purple-100\">This version uses <strong>History API</strong> for cleaner URLs (no # symbol). Compare with the hash-based version at <a href=\"/emscripten-c-spa\" class=\"underline\">/emscripten-c-spa</a></p>\n"
        "      </div>\n"
        "    </div>\n"
        "    <div class=\"text-center mb-12\">\n"
        "      <h1 class=\"text-5xl font-bold text-gray-900 dark:text-white mb-4\">\n"
        "        Welcome to C/WASM SPA\n"
        "      </h1>\n"
        "      <p class=\"text-xl text-gray-600 dark:text-gray-300 max-w-2xl mx-auto\">\n"
        "        Multi-page SPA with <span class=\"font-semibold text-purple-600 dark:text-purple-400\">clean URLs</span> using the History API\n"
        "      </p>\n"
        "    </div>\n"
        "    <div class=\"grid grid-cols-1 md:grid-cols-2 gap-6 max-w-4xl mx-auto\">\n"
        "      <a href=\"/emscripten-c-spa-alt/about\" data-route=\"/about\" class=\"block bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6 hover:shadow-md transition-shadow\">\n"
        "        <div class=\"flex items-center gap-3 mb-3\">\n"
        "          <svg class=\"w-6 h-6 text-purple-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z\"></path>\n"
        "          </svg>\n"
        "          <h3 class=\"text-lg font-semibold text-gray-900 dark:text-white\">About</h3>\n"
        "        </div>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Learn about the History API routing</p>\n"
        "      </a>\n"
        "      <a href=\"/emscripten-c-spa-alt/counter\" data-route=\"/counter\" class=\"block bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6 hover:shadow-md transition-shadow\">\n"
        "        <div class=\"flex items-center gap-3 mb-3\">\n"
        "          <svg class=\"w-6 h-6 text-green-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M7 11l5-5m0 0l5 5m-5-5v12\"></path>\n"
        "          </svg>\n"
        "          <h3 class=\"text-lg font-semibold text-gray-900 dark:text-white\">Interactive Counter</h3>\n"
        "        </div>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Try the WASM-powered counter</p>\n"
        "      </a>\n"
        "      <a href=\"/emscripten-c-spa-alt/services\" data-route=\"/services\" class=\"block bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6 hover:shadow-md transition-shadow\">\n"
        "        <div class=\"flex items-center gap-3 mb-3\">\n"
        "          <svg class=\"w-6 h-6 text-orange-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M4 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2V6zM14 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2V6zM4 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2v-2zM14 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2v-2z\"></path>\n"
        "          </svg>\n"
        "          <h3 class=\"text-lg font-semibold text-gray-900 dark:text-white\">Services</h3>\n"
        "        </div>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Explore capabilities</p>\n"
        "      </a>\n"
        "      <div class=\"bg-gradient-to-br from-purple-50 to-pink-50 dark:from-purple-900/20 dark:to-pink-900/20 rounded-lg shadow-sm border border-purple-200 dark:border-purple-800 p-6\">\n"
        "        <div class=\"flex items-center gap-3 mb-3\">\n"
        "          <svg class=\"w-6 h-6 text-yellow-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "          <h3 class=\"text-lg font-semibold text-gray-900 dark:text-white\">Current Counter</h3>\n"
        "        </div>\n"
        "        <p class=\"text-3xl font-bold text-purple-600 dark:text-purple-400\">%d</p>\n"
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
    js_set_title("About - C/WASM SPA (History API)");

    char html[8192];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-purple-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "          <div>\n"
        "            <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "            <p class=\"text-xs text-purple-600 dark:text-purple-400\">History API Mode</p>\n"
        "          </div>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"/emscripten-c-spa-alt/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Home</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors bg-purple-100 dark:bg-purple-900/30 text-purple-700 dark:text-purple-300\">About</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Counter</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-4xl mx-auto px-4 sm:px-6 lg:px-8 py-12\">\n"
        "    <h1 class=\"text-4xl font-bold text-gray-900 dark:text-white mb-6\">About History API Routing</h1>\n"
        "    <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-8 mb-6\">\n"
        "      <h2 class=\"text-2xl font-semibold text-gray-900 dark:text-white mb-4\">History API vs Hash-Based</h2>\n"
        "      <div class=\"grid grid-cols-1 md:grid-cols-2 gap-6\">\n"
        "        <div class=\"bg-purple-50 dark:bg-purple-900/20 rounded-lg p-4 border border-purple-200 dark:border-purple-800\">\n"
        "          <div class=\"text-purple-600 dark:text-purple-400 text-sm font-medium mb-2\">History API (This Version)</div>\n"
        "          <div class=\"text-sm text-gray-700 dark:text-gray-300 space-y-1\">\n"
        "            <div>✓ Clean URLs: <code class=\"bg-purple-100 dark:bg-purple-900/40 px-1 rounded\">/about</code></div>\n"
        "            <div>✓ Professional appearance</div>\n"
        "            <div>✓ SEO-friendly</div>\n"
        "            <div>✓ Uses <code class=\"bg-purple-100 dark:bg-purple-900/40 px-1 rounded\">history.pushState()</code></div>\n"
        "          </div>\n"
        "        </div>\n"
        "        <div class=\"bg-blue-50 dark:bg-blue-900/20 rounded-lg p-4 border border-blue-200 dark:border-blue-800\">\n"
        "          <div class=\"text-blue-600 dark:text-blue-400 text-sm font-medium mb-2\">Hash-Based</div>\n"
        "          <div class=\"text-sm text-gray-700 dark:text-gray-300 space-y-1\">\n"
        "            <div>• URLs: <code class=\"bg-blue-100 dark:bg-blue-900/40 px-1 rounded\">#/about</code></div>\n"
        "            <div>• Simpler implementation</div>\n"
        "            <div>• No server config needed</div>\n"
        "            <div>• Uses <code class=\"bg-blue-100 dark:bg-blue-900/40 px-1 rounded\">window.location.hash</code></div>\n"
        "          </div>\n"
        "        </div>\n"
        "      </div>\n"
        "    </div>\n"
        "    <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-8 mb-6\">\n"
        "      <h2 class=\"text-2xl font-semibold text-gray-900 dark:text-white mb-4\">Implementation Details</h2>\n"
        "      <ul class=\"space-y-3\">\n"
        "        <li class=\"flex items-start gap-3\">\n"
        "          <svg class=\"w-6 h-6 text-purple-500 flex-shrink-0 mt-0.5\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M5 13l4 4L19 7\"></path>\n"
        "          </svg>\n"
        "          <span class=\"text-gray-700 dark:text-gray-300\"><strong>Popstate events</strong> - Listens for browser back/forward button clicks</span>\n"
        "        </li>\n"
        "        <li class=\"flex items-start gap-3\">\n"
        "          <svg class=\"w-6 h-6 text-purple-500 flex-shrink-0 mt-0.5\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M5 13l4 4L19 7\"></path>\n"
        "          </svg>\n"
        "          <span class=\"text-gray-700 dark:text-gray-300\"><strong>Link interception</strong> - Prevents full page reloads on navigation</span>\n"
        "        </li>\n"
        "        <li class=\"flex items-start gap-3\">\n"
        "          <svg class=\"w-6 h-6 text-purple-500 flex-shrink-0 mt-0.5\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M5 13l4 4L19 7\"></path>\n"
        "          </svg>\n"
        "          <span class=\"text-gray-700 dark:text-gray-300\"><strong>Astro catch-all</strong> - Server routes all paths to the same page</span>\n"
        "        </li>\n"
        "      </ul>\n"
        "    </div>\n"
        "    <div class=\"bg-gradient-to-br from-purple-50 to-blue-50 dark:from-purple-900/20 dark:to-blue-900/20 rounded-lg border border-purple-200 dark:border-purple-800 p-8\">\n"
        "      <h2 class=\"text-2xl font-semibold text-gray-900 dark:text-white mb-4\">Try It Out</h2>\n"
        "      <p class=\"text-gray-700 dark:text-gray-300 mb-4\">\n"
        "        Notice how the URL changes without the # symbol when you navigate. The browser's back and forward buttons work seamlessly!\n"
        "      </p>\n"
        "      <p class=\"text-gray-700 dark:text-gray-300\">\n"
        "        Compare with the <a href=\"/emscripten-c-spa\" class=\"text-purple-600 dark:text-purple-400 underline font-semibold\">hash-based version</a> to see the difference.\n"
        "      </p>\n"
        "    </div>\n"
        "  </main>\n"
        "</div>"
    );

    js_set_content(html);
    printf("📄 Rendered: About page\n");
}

void render_counter() {
    js_set_title("Counter - C/WASM SPA (History API)");

    char html[8192];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-purple-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "          <div>\n"
        "            <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "            <p class=\"text-xs text-purple-600 dark:text-purple-400\">History API Mode</p>\n"
        "          </div>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"/emscripten-c-spa-alt/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Home</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">About</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors bg-purple-100 dark:bg-purple-900/30 text-purple-700 dark:text-purple-300\">Counter</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-4xl mx-auto px-4 sm:px-6 lg:px-8 py-12\">\n"
        "    <h1 class=\"text-4xl font-bold text-gray-900 dark:text-white mb-8 text-center\">Interactive Counter</h1>\n"
        "    <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-8\">\n"
        "      <div class=\"flex flex-col items-center gap-8\">\n"
        "        <div class=\"text-center\">\n"
        "          <div class=\"text-8xl font-bold text-purple-600 dark:text-purple-400\" id=\"counter-value\">%d</div>\n"
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
        "            Notice the clean URLs without # symbols as you navigate!\n"
        "          </p>\n"
        "        </div>\n"
        "      </div>\n"
        "    </div>\n"
        "  </main>\n"
        "</div>",
        counter
    );

    js_set_content(html);
    printf("📄 Rendered: Counter page (value: %d)\n", counter);
}

void render_services() {
    js_set_title("Services - C/WASM SPA (History API)");

    char html[8192];
    snprintf(html, sizeof(html),
        "<div class=\"min-h-screen bg-gray-50 dark:bg-zinc-900\">\n"
        "  <header class=\"bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 sticky top-0 z-50\">\n"
        "    <div class=\"max-w-7xl mx-auto px-4 sm:px-6 lg:px-8\">\n"
        "      <div class=\"flex items-center justify-between h-16\">\n"
        "        <div class=\"flex items-center gap-3\">\n"
        "          <svg class=\"w-8 h-8 text-purple-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "          <div>\n"
        "            <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "            <p class=\"text-xs text-purple-600 dark:text-purple-400\">History API Mode</p>\n"
        "          </div>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"/emscripten-c-spa-alt/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Home</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">About</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Counter</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors bg-purple-100 dark:bg-purple-900/30 text-purple-700 dark:text-purple-300\">Services</a>\n"
        "        </nav>\n"
        "      </div>\n"
        "    </div>\n"
        "  </header>\n"
        "  <main class=\"max-w-6xl mx-auto px-4 sm:px-6 lg:px-8 py-12\">\n"
        "    <h1 class=\"text-4xl font-bold text-gray-900 dark:text-white mb-4 text-center\">History API Features</h1>\n"
        "    <p class=\"text-xl text-gray-600 dark:text-gray-300 text-center mb-12 max-w-3xl mx-auto\">\n"
        "      Modern SPA routing with clean, professional URLs\n"
        "    </p>\n"
        "    <div class=\"grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6\">\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-purple-100 dark:bg-purple-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-purple-600 dark:text-purple-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">Clean URLs</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">No hash symbols - URLs look like traditional server-routed apps</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-blue-100 dark:bg-blue-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-blue-600 dark:text-blue-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M10 20l4-16m4 4l4 4-4 4M6 16l-4-4 4-4\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">Popstate Events</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Perfect browser back/forward button integration</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-green-100 dark:bg-green-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-green-600 dark:text-green-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">SEO Friendly</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Search engines can index clean URLs more easily</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-yellow-100 dark:bg-yellow-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-yellow-600 dark:text-yellow-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M4 7v10c0 2.21 3.582 4 8 4s8-1.79 8-4V7M4 7c0 2.21 3.582 4 8 4s8-1.79 8-4M4 7c0-2.21 3.582-4 8-4s8 1.79 8 4\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">State Management</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Application state persists perfectly across navigation</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-red-100 dark:bg-red-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-red-600 dark:text-red-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">Astro Integration</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">Catch-all routes handle all paths seamlessly</p>\n"
        "      </div>\n"
        "      <div class=\"bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6\">\n"
        "        <div class=\"w-12 h-12 bg-indigo-100 dark:bg-indigo-900/30 rounded-lg flex items-center justify-center mb-4\">\n"
        "          <svg class=\"w-6 h-6 text-indigo-600 dark:text-indigo-400\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M7 21a4 4 0 01-4-4V5a2 2 0 012-2h4a2 2 0 012 2v12a4 4 0 01-4 4zm0 0h12a2 2 0 002-2v-4a2 2 0 00-2-2h-2.343M11 7.343l1.657-1.657a2 2 0 012.828 0l2.829 2.829a2 2 0 010 2.828l-8.486 8.485M7 17h.01\"></path>\n"
        "          </svg>\n"
        "        </div>\n"
        "        <h3 class=\"text-xl font-semibold text-gray-900 dark:text-white mb-2\">Pure C/WASM</h3>\n"
        "        <p class=\"text-gray-600 dark:text-gray-300\">All routing logic written in C, compiled to WebAssembly</p>\n"
        "      </div>\n"
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
        "          <svg class=\"w-8 h-8 text-purple-500\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
        "            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"></path>\n"
        "          </svg>\n"
        "          <div>\n"
        "            <h1 class=\"text-xl font-bold text-gray-900 dark:text-white\">C/WASM SPA</h1>\n"
        "            <p class=\"text-xs text-purple-600 dark:text-purple-400\">History API Mode</p>\n"
        "          </div>\n"
        "        </div>\n"
        "        <nav class=\"flex gap-1\">\n"
        "          <a href=\"/emscripten-c-spa-alt/home\" data-route=\"/home\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Home</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/about\" data-route=\"/about\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">About</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/counter\" data-route=\"/counter\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Counter</a>\n"
        "          <a href=\"/emscripten-c-spa-alt/services\" data-route=\"/services\" class=\"px-4 py-2 rounded-lg text-sm font-medium transition-colors text-gray-600 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-zinc-700\">Services</a>\n"
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
        "      The page you're looking for doesn't exist.\n"
        "    </p>\n"
        "    <a href=\"/emscripten-c-spa-alt/home\" data-route=\"/home\" class=\"inline-block px-6 py-3 bg-purple-600 hover:bg-purple-700 text-white font-medium rounded-lg transition-colors\">\n"
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
// Router - History API
// ======================

EMSCRIPTEN_KEEPALIVE
void handle_route_change() {
    js_get_pathname(current_route, sizeof(current_route));

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
    js_push_state(path);
}

// ======================
// Application Entry Point
// ======================

int main() {
    printf("🚀 C/WASM Multi-Page SPA (History API) Starting...\n");
    printf("📦 Setting up History API routing system\n");

    // Set up popstate listener for browser navigation
    js_setup_popstate_listener();

    // Route to initial page
    handle_route_change();

    printf("✅ Application initialized successfully!\n");
    printf("🌐 Available routes: /home, /about, /counter, /services\n");
    printf("✨ Using History API for clean URLs\n");

    return 0;
}
