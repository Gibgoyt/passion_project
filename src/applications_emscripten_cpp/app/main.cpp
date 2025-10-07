#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <string>
#include <cstdio>

// Counter state (using C++ namespace)
namespace {
    int counter = 0;
}

// Update the counter display
void update_counter_display() {
    char buffer[50];
    std::snprintf(buffer, sizeof(buffer), "%d", counter);

    EM_ASM({
        const counterEl = document.getElementById('counter-value');
        if (counterEl) {
            counterEl.textContent = UTF8ToString($0);
        }
    }, buffer);
}

// Increment counter
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void increment_counter() {
        counter++;
        update_counter_display();
        std::cout << "Counter incremented to: " << counter << std::endl;
    }

    // Decrement counter
    EMSCRIPTEN_KEEPALIVE
    void decrement_counter() {
        counter--;
        update_counter_display();
        std::cout << "Counter decremented to: " << counter << std::endl;
    }

    // Reset counter
    EMSCRIPTEN_KEEPALIVE
    void reset_counter() {
        counter = 0;
        update_counter_display();
        std::cout << "Counter reset to: 0" << std::endl;
    }

    // Initialize the dashboard
    EMSCRIPTEN_KEEPALIVE
    void init_dashboard() {
        std::cout << "🚀 Emscripten C++ Dashboard initialized!" << std::endl;

        // Create the dashboard HTML structure
        EM_ASM({
            const root = document.getElementById('wasm-root');
            if (!root) {
                console.error('No #wasm-root element found');
                return;
            }

            root.innerHTML = `
                <div class="min-h-screen bg-gray-50 dark:bg-zinc-900">
                    \x3c!-- Header --\x3e
                    <header class="bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700">
                        <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
                            <div class="flex items-center justify-between">
                                <div class="flex items-center gap-3">
                                    <svg class="w-8 h-8 text-purple-500" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M10 20l4-16m4 4l4 4-4 4M6 16l-4-4 4-4"></path>
                                    </svg>
                                    <h1 class="text-2xl font-bold text-gray-900 dark:text-white">
                                        Emscripten C++ Dashboard
                                    </h1>
                                </div>
                                <div class="text-sm text-gray-500 dark:text-gray-400">
                                    WebAssembly + C++
                                </div>
                            </div>
                        </div>
                    </header>

                    \x3c!-- Main Content --\x3e
                    <main class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
                        \x3c!-- Welcome Card --\x3e
                        <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6 mb-6">
                            <h2 class="text-xl font-semibold text-gray-900 dark:text-white mb-2">
                                Welcome to the Emscripten C++ Dashboard
                            </h2>
                            <p class="text-gray-600 dark:text-gray-300">
                                This is a simple dashboard built with C++ and compiled to WebAssembly using Emscripten.
                                The UI uses Tailwind CSS for styling with C++ STL features.
                            </p>
                        </div>

                        \x3c!-- Counter Section --\x3e
                        <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6">
                            <h2 class="text-xl font-semibold text-gray-900 dark:text-white mb-4">
                                Interactive Counter
                            </h2>

                            <div class="flex flex-col items-center gap-6">
                                \x3c!-- Counter Display --\x3e
                                <div class="text-center">
                                    <div class="text-6xl font-bold text-purple-600 dark:text-purple-400" id="counter-value">
                                        0
                                    </div>
                                    <div class="text-sm text-gray-500 dark:text-gray-400 mt-2">
                                        Current Count
                                    </div>
                                </div>

                                \x3c!-- Counter Controls --\x3e
                                <div class="flex gap-4">
                                    <button
                                        onclick="Module._decrement_counter()"
                                        class="px-6 py-3 bg-red-500 hover:bg-red-600 text-white font-medium rounded-lg transition-colors shadow-sm"
                                    >
                                        <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M20 12H4"></path>
                                        </svg>
                                    </button>

                                    <button
                                        onclick="Module._reset_counter()"
                                        class="px-6 py-3 bg-gray-500 hover:bg-gray-600 text-white font-medium rounded-lg transition-colors shadow-sm"
                                    >
                                        Reset
                                    </button>

                                    <button
                                        onclick="Module._increment_counter()"
                                        class="px-6 py-3 bg-green-500 hover:bg-green-600 text-white font-medium rounded-lg transition-colors shadow-sm"
                                    >
                                        <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4"></path>
                                        </svg>
                                    </button>
                                </div>

                                \x3c!-- Info --\x3e
                                <div class="text-sm text-gray-500 dark:text-gray-400 text-center max-w-md">
                                    This counter is managed entirely by C++ code running in WebAssembly.
                                    Click the buttons to interact with the WASM module.
                                </div>
                            </div>
                        </div>

                        \x3c!-- Stats Grid --\x3e
                        <div class="grid grid-cols-1 md:grid-cols-3 gap-6 mt-6">
                            <div class="bg-purple-50 dark:bg-purple-900/20 rounded-lg p-6 border border-purple-200 dark:border-purple-800">
                                <div class="text-purple-600 dark:text-purple-400 text-sm font-medium mb-1">
                                    Language
                                </div>
                                <div class="text-2xl font-bold text-purple-900 dark:text-purple-100">
                                    C++
                                </div>
                            </div>

                            <div class="bg-blue-50 dark:bg-blue-900/20 rounded-lg p-6 border border-blue-200 dark:border-blue-800">
                                <div class="text-blue-600 dark:text-blue-400 text-sm font-medium mb-1">
                                    Compiler
                                </div>
                                <div class="text-2xl font-bold text-blue-900 dark:text-blue-100">
                                    em++ (Emscripten)
                                </div>
                            </div>

                            <div class="bg-green-50 dark:bg-green-900/20 rounded-lg p-6 border border-green-200 dark:border-green-800">
                                <div class="text-green-600 dark:text-green-400 text-sm font-medium mb-1">
                                    Runtime
                                </div>
                                <div class="text-2xl font-bold text-green-900 dark:text-green-100">
                                    WebAssembly
                                </div>
                            </div>
                        </div>
                    </main>
                </div>
            `;

            console.log('✅ Dashboard HTML created successfully');
        });

        // Initialize the counter display
        update_counter_display();
    }
}

// Main entry point
int main() {
    std::cout << "Emscripten C++ WASM App - Starting..." << std::endl;
    init_dashboard();
    return 0;
}
