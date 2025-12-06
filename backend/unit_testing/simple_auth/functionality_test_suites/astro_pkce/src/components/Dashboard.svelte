<script>
  import { onMount } from 'svelte'
  import { Endpoints } from '@ts-testing/endpoints/index.ts'

  let user = null
  let loading = true
  let validating = false
  let refreshing = false

  onMount(async () => {
    const accessToken = localStorage.getItem('access_token')
    const userInfo = localStorage.getItem('user_info')

    if (!accessToken) {
      window.location.href = '/login'
      return
    }

    user = JSON.parse(userInfo || '{}')
    loading = false
  })

  async function validateToken() {
    validating = true
    try {
      const accessToken = localStorage.getItem('access_token')
      if (!accessToken) {
        alert('❌ No access token found')
        return
      }

      console.log('Validating access token...')
      const result = await Endpoints.Auth.Validate.POST(accessToken)

      console.log('Validation result:', JSON.stringify(result, null, 2))

      if (result.status === 200 && result.data.valid) {
        alert(`✅ Token is valid!\nUser: ${result.data.email}\nID: ${result.data.userId}`)
      } else {
        alert(`❌ Token validation failed: ${result.data.error || 'Invalid token'}`)
      }
    } catch (error) {
      console.error('Validation error:', error)
      alert('❌ Validation error: ' + error.message)
    } finally {
      validating = false
    }
  }

  async function refreshToken() {
    refreshing = true
    try {
      const refreshTokenValue = localStorage.getItem('refresh_token')
      if (!refreshTokenValue) {
        alert('❌ No refresh token found')
        return
      }

      console.log('Refreshing access token...')
      console.log('REFRESH TOKEN VALUE: ' + refreshTokenValue)
      const result = await Endpoints.Auth.Refresh.POST(refreshTokenValue)

      console.log('Refresh result:', JSON.stringify(result, null, 2))

      if (result.status === 200 && result.data.success) {
        // Update stored tokens
        localStorage.setItem('access_token', result.data.accessToken)
        localStorage.setItem('refresh_token', result.data.refreshToken)

        alert('✅ Tokens refreshed successfully!')
      } else {
        alert(`❌ Token refresh failed: ${result.data.error || 'Refresh failed'}`)
      }
    } catch (error) {
      console.error('Refresh error:', error)
      alert('❌ Refresh error: ' + error.message)
    } finally {
      refreshing = false
    }
  }

  function logout() {
    localStorage.clear()
    window.location.href = '/'
  }
</script>

{#if loading}
  <div class="flex items-center justify-center min-h-64">
    <div class="spinner-oauth mr-2"></div>
    <span class="text-gray-600">Loading dashboard...</span>
  </div>
{:else if user}
  <div class="max-w-6xl mx-auto space-y-6">
    <!-- Welcome Section -->
    <div class="card-oauth">
      <div class="flex items-start justify-between">
        <div>
          <h1 class="text-3xl font-bold text-gray-900 mb-2">Welcome back!</h1>
          <p class="text-lg text-gray-600 mb-4">
            You're successfully authenticated as <strong class="text-brand-600">{user.email}</strong>
          </p>
          <p class="text-sm text-gray-500">
            Using OAuth 2.1 with PKCE for secure authentication
          </p>
        </div>
        <div class="flex items-center space-x-2 rounded-lg bg-emerald-100 px-3 py-2">
          <div class="h-2 w-2 rounded-full bg-emerald-500"></div>
          <span class="text-sm font-medium text-emerald-700">Authenticated</span>
        </div>
      </div>
    </div>

    <!-- User Information -->
    <div class="grid grid-cols-1 lg:grid-cols-2 gap-6">
      <div class="card-oauth">
        <h2 class="text-xl font-semibold text-gray-900 mb-4 flex items-center">
          <svg class="h-5 w-5 mr-2 text-brand-600" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
            <path stroke-linecap="round" stroke-linejoin="round" d="M15.75 6a3.75 3.75 0 1 1-7.5 0 3.75 3.75 0 0 1 7.5 0ZM4.501 20.118a7.5 7.5 0 0 1 14.998 0A17.933 17.933 0 0 1 12 21.75c-2.676 0-5.216-.584-7.499-1.632Z" />
          </svg>
          User Profile
        </h2>

        <div class="space-y-4">
          <div class="flex justify-between py-2 border-b border-gray-100">
            <span class="text-sm font-medium text-gray-600">Email</span>
            <span class="text-sm text-gray-900">{user.email}</span>
          </div>

          <div class="flex justify-between py-2 border-b border-gray-100">
            <span class="text-sm font-medium text-gray-600">User ID</span>
            <code class="text-sm bg-gray-100 px-2 py-1 rounded text-gray-900">{user.userId}</code>
          </div>

          <div class="flex justify-between py-2 border-b border-gray-100">
            <span class="text-sm font-medium text-gray-600">Email Verified</span>
            {#if user.emailVerified}
              <div class="flex items-center space-x-1">
                <svg class="h-4 w-4 text-emerald-600" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
                  <path stroke-linecap="round" stroke-linejoin="round" d="M9 12.75L11.25 15 15 9.75M21 12a9 9 0 1 1-18 0 9 9 0 0 1 18 0Z" />
                </svg>
                <span class="text-sm text-emerald-600">Verified</span>
              </div>
            {:else}
              <div class="flex items-center space-x-1">
                <svg class="h-4 w-4 text-amber-600" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
                  <path stroke-linecap="round" stroke-linejoin="round" d="M12 9v3.75m9-.75a9 9 0 1 1-18 0 9 9 0 0 1 18 0Zm-9 3.75h.008v.008H12v-.008Z" />
                </svg>
                <span class="text-sm text-amber-600">Pending</span>
              </div>
            {/if}
          </div>

          <div class="flex justify-between py-2">
            <span class="text-sm font-medium text-gray-600">Session Started</span>
            <span class="text-sm text-gray-900">{new Date().toLocaleString()}</span>
          </div>
        </div>
      </div>

      <!-- Token Management -->
      <div class="card-oauth">
        <h3 class="text-lg font-semibold text-gray-900 mb-4 flex items-center">
          <svg class="h-5 w-5 mr-2 text-brand-600" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
            <path stroke-linecap="round" stroke-linejoin="round" d="M15.75 5.25a3 3 0 0 1 3 3m3 0a6 6 0 0 1-7.029 5.912c-.563-.097-1.159-.026-1.658.33L10.5 16.5l-1.072-1.072A3.75 3.75 0 0 1 5.25 12V8.25m0 0a3 3 0 0 1 3-3h3.75a3 3 0 0 1 3 3v8.25m-9 0V9a1.5 1.5 0 0 1 1.5-1.5h1.5A1.5 1.5 0 0 1 9 9v3.25" />
          </svg>
          Token Management
        </h3>

        <div class="space-y-3">
          <button on:click={validateToken} disabled={validating} class="btn-oauth-secondary w-full">
            {#if validating}
              <div class="spinner-oauth mr-2"></div>
              Validating...
            {:else}
              <svg class="h-4 w-4 mr-2" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
                <path stroke-linecap="round" stroke-linejoin="round" d="M9 12.75L11.25 15 15 9.75m-3.75 6.75H21a3 3 0 0 0 3-3V6a3 3 0 0 0-3-3H5.25A2.25 2.25 0 0 0 3 5.25v13.5A2.25 2.25 0 0 0 5.25 21h10.5a2.25 2.25 0 0 0 2.25-2.25V15" />
              </svg>
              Validate Token
            {/if}
          </button>

          <button on:click={refreshToken} disabled={refreshing} class="btn-oauth-secondary w-full">
            {#if refreshing}
              <div class="spinner-oauth mr-2"></div>
              Refreshing...
            {:else}
              <svg class="h-4 w-4 mr-2" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
                <path stroke-linecap="round" stroke-linejoin="round" d="M16.023 9.348h4.992v-.001M2.985 19.644v-4.992m0 0h4.992m-4.993 0 3.181 3.183a8.25 8.25 0 0 0 13.803-3.7M4.031 9.865a8.25 8.25 0 0 0 13.803-3.7l3.181 3.182m0-4.991v4.99" />
              </svg>
              Refresh Token
            {/if}
          </button>

          <button on:click={logout} class="btn-oauth-secondary w-full !text-red-600 !border-red-200 hover:!bg-red-50">
            <svg class="h-4 w-4 mr-2" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
              <path stroke-linecap="round" stroke-linejoin="round" d="M15.75 9V5.25A2.25 2.25 0 0 0 13.5 3h-6a2.25 2.25 0 0 0-2.25 2.25v13.5A2.25 2.25 0 0 0 7.5 21h6a2.25 2.25 0 0 0 2.25-2.25V15M12 9l-3 3m0 0 3 3m-3-3h12.75" />
            </svg>
            Logout
          </button>
        </div>
      </div>
    </div>

    <!-- PKCE Flow Information -->
    <div class="card-oauth">
      <h3 class="text-lg font-semibold text-gray-900 mb-4 flex items-center">
        <svg class="h-5 w-5 mr-2 text-brand-600" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor">
          <path stroke-linecap="round" stroke-linejoin="round" d="M16.5 10.5V6.75a4.5 4.5 0 1 0-9 0v3.75m-.75 11.25h10.5a2.25 2.25 0 0 0 2.25-2.25v-6.75a2.25 2.25 0 0 0-2.25-2.25H6.75a2.25 2.25 0 0 0-2.25 2.25v6.75a2.25 2.25 0 0 0 2.25 2.25Z" />
        </svg>
        OAuth 2.1 PKCE Flow Completed
      </h3>

      <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <div class="flex items-start space-x-3">
          <div class="flex h-6 w-6 items-center justify-center rounded-full bg-emerald-100 text-emerald-600 text-xs font-bold">
            ✓
          </div>
          <div>
            <p class="font-medium text-gray-900 text-sm">PKCE Parameters</p>
            <p class="text-gray-600 text-xs">Generated securely</p>
          </div>
        </div>

        <div class="flex items-start space-x-3">
          <div class="flex h-6 w-6 items-center justify-center rounded-full bg-emerald-100 text-emerald-600 text-xs font-bold">
            ✓
          </div>
          <div>
            <p class="font-medium text-gray-900 text-sm">Code Exchange</p>
            <p class="text-gray-600 text-xs">PKCE verified</p>
          </div>
        </div>

        <div class="flex items-start space-x-3">
          <div class="flex h-6 w-6 items-center justify-center rounded-full bg-emerald-100 text-emerald-600 text-xs font-bold">
            ✓
          </div>
          <div>
            <p class="font-medium text-gray-900 text-sm">Token Validation</p>
            <p class="text-gray-600 text-xs">Access granted</p>
          </div>
        </div>

        <div class="flex items-start space-x-3">
          <div class="flex h-6 w-6 items-center justify-center rounded-full bg-emerald-100 text-emerald-600 text-xs font-bold">
            ✓
          </div>
          <div>
            <p class="font-medium text-gray-900 text-sm">Client Storage</p>
            <p class="text-gray-600 text-xs">Tokens secured</p>
          </div>
        </div>
      </div>
    </div>
  </div>
{/if}
