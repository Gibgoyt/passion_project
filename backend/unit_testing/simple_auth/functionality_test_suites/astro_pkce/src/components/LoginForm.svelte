<script>
  // Import existing ts_testing endpoints - exactly like main.ts!
  import { Endpoints } from '@ts-testing/endpoints/index.ts'

  let email = 'testuser1@example.com'
  let password = 'Testuser1!'
  let loading = false
  let message = ''
  let currentStep = 0

  const steps = [
    'Generate PKCE parameters',
    'Initialize OAuth flow',
    'User authentication',
    'Code exchange',
    'Token validation'
  ]

  // Copy PKCE utilities from main.ts (browser-compatible versions)
  function generateCodeVerifier() {
    const array = new Uint8Array(32)
    crypto.getRandomValues(array)
    return base64URLEncode(array)
  }

  async function generateCodeChallenge(verifier) {
    const encoder = new TextEncoder()
    const data = encoder.encode(verifier)
    const digest = await crypto.subtle.digest('SHA-256', data)
    return base64URLEncode(new Uint8Array(digest))
  }

  function generateState() {
    const array = new Uint8Array(32)
    crypto.getRandomValues(array)
    return base64URLEncode(array)
  }

  // Browser-compatible base64URL (no Buffer)
  function base64URLEncode(array) {
    let binary = ''
    for (let i = 0; i < array.byteLength; i++) {
      binary += String.fromCharCode(array[i])
    }
    return btoa(binary).replace(/\+/g, '-').replace(/\//g, '_').replace(/=/g, '')
  }

  async function performPKCEFlow() {
    loading = true
    message = ''

    try {
      console.log('🧪 Starting OAuth 2.1 PKCE flow...')

      // Step 1: Generate PKCE parameters
      currentStep = 0
      await new Promise(r => setTimeout(r, 400))
      const codeVerifier = generateCodeVerifier()
      const codeChallenge = await generateCodeChallenge(codeVerifier)
      const state = generateState()

      console.log('📋 PKCE Parameters generated:')
      console.log(`   Code verifier: ${codeVerifier.substring(0, 10)}...`)
      console.log(`   Code challenge: ${codeChallenge.substring(0, 10)}...`)
      console.log(`   State: ${state.substring(0, 10)}...`)

      // Step 2: Use existing endpoints - exactly like main.ts!
      currentStep = 1
      await new Promise(r => setTimeout(r, 400))
      console.log('SENDING POST /oauth/authorize/init')

      const initResult = await Endpoints.OAuth.AuthorizeInit.POST(
        "astro-pkce-test",
        "http://localhost:3000/callback",
        codeChallenge,
        "S256",
        state
      )

      console.log('POST /oauth/authorize/init response:', JSON.stringify(initResult, null, 2))

      if (initResult.status !== 200) {
        throw new Error(`OAuth initialization failed: ${initResult.data.error || 'Unknown error'}`)
      }

      console.log('✅ PKCE initialization successful!')
      console.log(`   Session ID: ${initResult.data.session_id}`)

      // Step 3: Complete authorization
      currentStep = 2
      await new Promise(r => setTimeout(r, 400))
      console.log('SENDING POST /oauth/authorize/complete')

      const completeResult = await Endpoints.OAuth.AuthorizeComplete.POST(
        initResult.data.session_id,
        email,
        password,
        true
      )

      console.log('POST /oauth/authorize/complete response:', JSON.stringify(completeResult, null, 2))

      if (completeResult.status !== 200) {
        throw new Error(`User authorization failed: ${completeResult.data.error || 'Unknown error'}`)
      }

      console.log('✅ User authorization successful!')
      console.log(`   Authorization code: ${completeResult.data.authorization_code.substring(0, 10)}...`)

      // Step 4: Exchange code for tokens
      currentStep = 3
      await new Promise(r => setTimeout(r, 400))
      console.log('SENDING POST /oauth/token')

      const tokenResult = await Endpoints.OAuth.Token.POST(
        "authorization_code",
        completeResult.data.authorization_code,
        codeVerifier,
        "astro-pkce-test",
        "http://localhost:3000/callback"
      )

      console.log('POST /oauth/token response:', JSON.stringify(tokenResult, null, 2))

      if (tokenResult.status !== 200) {
        throw new Error(`Token exchange failed: ${tokenResult.data.error || 'Unknown error'}`)
      }

      console.log('✅ Token exchange successful!')
      console.log(`   Access token: ${tokenResult.data.access_token.substring(0, 20)}...`)
      console.log(`   Refresh token: ${tokenResult.data.refresh_token.substring(0, 20)}...`)

      // Step 5: Store tokens client-side
      currentStep = 4
      await new Promise(r => setTimeout(r, 400))

      // Validate the token first
      console.log('SENDING POST /auth/validate')
      const validateResult = await Endpoints.Auth.Validate.POST(tokenResult.data.access_token)

      console.log('POST /auth/validate response:', JSON.stringify(validateResult, null, 2))

      if (validateResult.status !== 200) {
        throw new Error(`Token validation failed: ${validateResult.data.error || 'Unknown error'}`)
      }

      console.log('✅ Token validation successful!')
      console.log(`   User: ${validateResult.data.email} (ID: ${validateResult.data.userId})`)

      // Store in localStorage
      localStorage.setItem('access_token', tokenResult.data.access_token)
      localStorage.setItem('refresh_token', tokenResult.data.refresh_token)
      localStorage.setItem('user_info', JSON.stringify({
        userId: validateResult.data.userId,
        email: validateResult.data.email,
        emailVerified: validateResult.data.emailVerified
      }))

      console.log('🎉 PKCE flow completed successfully!')
      message = 'Authentication successful! Redirecting...'
      setTimeout(() => window.location.href = '/app', 1500)

    } catch (error) {
      console.error('Login failed:', error)
      message = `Error: ${error.message}`
      currentStep = -1
    } finally {
      loading = false
    }
  }
</script>

<!-- Beautiful Tailwind v4 UI -->
<div class="grid grid-cols-1 lg:grid-cols-2 gap-6 max-w-4xl mx-auto">
  <div class="card-oauth">
    <h2 class="text-2xl font-bold text-gray-900 mb-6">Login with OAuth PKCE</h2>
    <p class="text-sm text-gray-600 mb-6">Secure authentication using OAuth 2.1 with PKCE</p>

    <form on:submit|preventDefault={performPKCEFlow} class="space-y-4">
      <div>
        <label for="email" class="block text-sm font-medium text-gray-700 mb-2">Email address</label>
        <input bind:value={email} type="email" id="email" class="input-oauth" required />
      </div>

      <div>
        <label for="password" class="block text-sm font-medium text-gray-700 mb-2">Password</label>
        <input bind:value={password} type="password" id="password" class="input-oauth" required />
      </div>

      <button type="submit" disabled={loading} class="btn-oauth w-full">
        {#if loading}
          <div class="spinner-oauth mr-2"></div>
          Authenticating...
        {:else}
          <svg class="h-5 w-5 mr-2" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
            <path stroke-linecap="round" stroke-linejoin="round" d="M16.5 10.5V6.75a4.5 4.5 0 1 0-9 0v3.75m-.75 11.25h10.5a2.25 2.25 0 0 0 2.25-2.25v-6.75a2.25 2.25 0 0 0-2.25-2.25H6.75a2.25 2.25 0 0 0-2.25 2.25v6.75a2.25 2.25 0 0 0 2.25 2.25Z" />
          </svg>
          Sign in with PKCE
        {/if}
      </button>

      {#if message}
        <div class="message-{message.includes('Error') ? 'error' : 'success'}">
          {message}
        </div>
      {/if}
    </form>
  </div>

  <div class="card-oauth">
    <h3 class="text-lg font-semibold text-gray-900 mb-4">Authentication Progress</h3>
    <div class="space-y-3">
      {#each steps as step, i}
        <div class="step-{i < currentStep ? 'completed' : i === currentStep ? 'active' : 'pending'}">
          <div class="flex items-center">
            <div class="flex h-6 w-6 items-center justify-center rounded-full {i < currentStep ? 'bg-emerald-500' : i === currentStep ? 'bg-brand-500' : 'bg-gray-400'} text-white text-xs font-bold mr-3">
              {#if i < currentStep}
                ✓
              {:else}
                {i + 1}
              {/if}
            </div>
            <span class="text-sm">{step}</span>
          </div>
        </div>
      {/each}
    </div>

    <div class="mt-6 p-4 bg-yellow-50 border border-yellow-200 rounded-lg">
      <p class="text-sm text-yellow-800">
        <strong>Test Credentials:</strong><br>
        Email: testuser1@example.com<br>
        Password: Testuser1!
      </p>
    </div>
  </div>
</div>