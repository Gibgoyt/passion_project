// Shim for using Node.js logic in browser
if (typeof window !== 'undefined') {
    // @ts-ignore
    window.process = window.process || {};
    // @ts-ignore
    window.process.env = window.process.env || {};
    
    // Default to the backend server URL if not specified
    // @ts-ignore
    if (!window.process.env.BASE_URL) {
        // @ts-ignore
        window.process.env.BASE_URL = 'https://localhost:2053';
    }

    // @ts-ignore
    window.process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"; // Ignored in browser but prevents crash on assignment
}