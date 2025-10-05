<script>
	// Form state
	let formData = {
		name: '',
		email: '',
		phone: '',
		company: '',
		service: '',
		message: ''
	};

	// UI state
	let isSubmitting = false;
	let errors = {};
	let successMessage = '';
	let errorMessage = '';

	// Service options
	const serviceOptions = [
		{ value: '', label: 'Select a service' },
		{ value: 'cybersecurity', label: 'Cybersecurity Solutions' },
		{ value: 'automation', label: 'System Automation' },
		{ value: 'cloud', label: 'Cloud & Data Services' },
		{ value: 'network', label: 'Network Management' },
		{ value: 'support', label: 'IT Support & Consulting' },
		{ value: 'development', label: 'Custom Software Development' }
	];

	// Validation functions
	function validateField(field, value) {
		switch (field) {
			case 'name':
				if (!value.trim()) return 'Full name is required';
				if (value.length > 100) return 'Name must be 100 characters or less';
				break;
			case 'email':
				if (!value.trim()) return 'Email address is required';
				if (value.length > 255) return 'Email must be 255 characters or less';
				// Basic email format validation
				const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
				if (!emailRegex.test(value.trim())) return 'Please enter a valid email address';
				break;
			case 'phone':
				if (!value.trim()) return 'Phone number is required';
				if (value.length > 20) return 'Phone number must be 20 characters or less';
				break;
			case 'message':
				if (!value.trim()) return 'Message is required';
				if (value.length > 1000) return 'Message must be 1000 characters or less';
				break;
		}
		return '';
	}

	function validateForm() {
		const newErrors = {};

		newErrors.name = validateField('name', formData.name);
		newErrors.email = validateField('email', formData.email);
		newErrors.phone = validateField('phone', formData.phone);
		newErrors.message = validateField('message', formData.message);

		// Remove empty error messages
		Object.keys(newErrors).forEach(key => {
			if (!newErrors[key]) delete newErrors[key];
		});

		errors = newErrors;
		return Object.keys(errors).length === 0;
	}

	// Real-time validation
	function handleInput(field, value) {
		formData[field] = value;
		// Clear error for this field when user starts typing
		if (errors[field]) {
			errors = { ...errors };
			delete errors[field];
		}
		// Clear messages when user starts typing
		if (successMessage) successMessage = '';
		if (errorMessage) errorMessage = '';
	}

	async function handleSubmit() {
		// Clear previous messages
		successMessage = '';
		errorMessage = '';

		// Validate form
		if (!validateForm()) {
			return;
		}

		isSubmitting = true;

		try {
			const response = await fetch('/api/contact-form', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(formData)
			});

			const result = await response.json();

			if (result.success) {
				successMessage = result.message || 'Thank you for your message! We will get back to you soon.';
				// Reset form
				formData = {
					name: '',
					email: '',
					phone: '',
					company: '',
					service: '',
					message: ''
				};
				errors = {};
			} else {
				errorMessage = result.error || 'Failed to send message. Please try again.';
			}
		} catch (error) {
			console.error('Form submission error:', error);
			errorMessage = 'An error occurred while sending your message. Please try again later.';
		} finally {
			isSubmitting = false;
		}
	}
</script>

<div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-8">
	<!-- Success Message -->
	{#if successMessage}
		<div class="mb-6 p-4 bg-green-100 dark:bg-green-900 border border-green-400 dark:border-green-600 text-green-700 dark:text-green-200 rounded-lg">
			{successMessage}
		</div>
	{/if}

	<!-- Error Message -->
	{#if errorMessage}
		<div class="mb-6 p-4 bg-red-100 dark:bg-red-900 border border-red-400 dark:border-red-600 text-red-700 dark:text-red-200 rounded-lg">
			{errorMessage}
		</div>
	{/if}

	<form on:submit|preventDefault={handleSubmit} class="space-y-6">
		<!-- Full Name -->
		<div>
			<label for="name" class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
				Full Name *
			</label>
			<input
				type="text"
				id="name"
				bind:value={formData.name}
				on:input={(e) => handleInput('name', e.target.value)}
				class="w-full px-4 py-3 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 dark:focus:ring-blue-400 dark:focus:border-blue-400 bg-white dark:bg-gray-700 text-gray-900 dark:text-gray-100 placeholder-gray-500 dark:placeholder-gray-400 transition-colors
				{errors.name ? 'border-red-500 dark:border-red-400' : 'border-gray-300 dark:border-gray-600'}"
				disabled={isSubmitting}
			/>
			{#if errors.name}
				<p class="mt-1 text-sm text-red-600 dark:text-red-400">{errors.name}</p>
			{/if}
		</div>

		<!-- Email Address -->
		<div>
			<label for="email" class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
				Email Address *
			</label>
			<input
				type="email"
				id="email"
				bind:value={formData.email}
				on:input={(e) => handleInput('email', e.target.value)}
				placeholder="your@email.com"
				class="w-full px-4 py-3 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 dark:focus:ring-blue-400 dark:focus:border-blue-400 bg-white dark:bg-gray-700 text-gray-900 dark:text-gray-100 placeholder-gray-500 dark:placeholder-gray-400 transition-colors
				{errors.email ? 'border-red-500 dark:border-red-400' : 'border-gray-300 dark:border-gray-600'}"
				disabled={isSubmitting}
			/>
			{#if errors.email}
				<p class="mt-1 text-sm text-red-600 dark:text-red-400">{errors.email}</p>
			{/if}
		</div>

		<!-- Phone Number -->
		<div>
			<label for="phone" class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
				Phone Number *
			</label>
			<input
				type="tel"
				id="phone"
				bind:value={formData.phone}
				on:input={(e) => handleInput('phone', e.target.value)}
				class="w-full px-4 py-3 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 dark:focus:ring-blue-400 dark:focus:border-blue-400 bg-white dark:bg-gray-700 text-gray-900 dark:text-gray-100 placeholder-gray-500 dark:placeholder-gray-400 transition-colors
				{errors.phone ? 'border-red-500 dark:border-red-400' : 'border-gray-300 dark:border-gray-600'}"
				disabled={isSubmitting}
			/>
			{#if errors.phone}
				<p class="mt-1 text-sm text-red-600 dark:text-red-400">{errors.phone}</p>
			{/if}
		</div>

		<!-- Company Name -->
		<div>
			<label for="company" class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
				Company Name
			</label>
			<input
				type="text"
				id="company"
				bind:value={formData.company}
				on:input={(e) => handleInput('company', e.target.value)}
				class="w-full px-4 py-3 border border-gray-300 dark:border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 dark:focus:ring-blue-400 dark:focus:border-blue-400 bg-white dark:bg-gray-700 text-gray-900 dark:text-gray-100 placeholder-gray-500 dark:placeholder-gray-400 transition-colors"
				disabled={isSubmitting}
			/>
		</div>

		<!-- Service Interest -->
		<div>
			<label for="service" class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
				Service Interest
			</label>
			<select
				id="service"
				bind:value={formData.service}
				on:change={(e) => handleInput('service', e.target.value)}
				class="w-full px-4 py-3 border border-gray-300 dark:border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 dark:focus:ring-blue-400 dark:focus:border-blue-400 bg-white dark:bg-gray-700 text-gray-900 dark:text-gray-100 transition-colors"
				disabled={isSubmitting}
			>
				{#each serviceOptions as option}
					<option value={option.value}>{option.label}</option>
				{/each}
			</select>
		</div>

		<!-- Message -->
		<div>
			<label for="message" class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
				Message *
			</label>
			<textarea
				id="message"
				bind:value={formData.message}
				on:input={(e) => handleInput('message', e.target.value)}
				rows="5"
				placeholder="Tell us about your project or how we can help..."
				class="w-full px-4 py-3 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 dark:focus:ring-blue-400 dark:focus:border-blue-400 bg-white dark:bg-gray-700 text-gray-900 dark:text-gray-100 placeholder-gray-500 dark:placeholder-gray-400 resize-vertical transition-colors
				{errors.message ? 'border-red-500 dark:border-red-400' : 'border-gray-300 dark:border-gray-600'}"
				disabled={isSubmitting}
			></textarea>
			{#if errors.message}
				<p class="mt-1 text-sm text-red-600 dark:text-red-400">{errors.message}</p>
			{/if}
		</div>

		<!-- Submit Button -->
		<button
			type="submit"
			disabled={isSubmitting}
			class="w-full px-8 py-4 bg-blue-600 text-white text-lg font-medium rounded-lg hover:bg-blue-700 focus:ring-4 focus:ring-blue-200 dark:focus:ring-blue-800 transition-colors shadow-lg hover:shadow-xl disabled:opacity-50 disabled:cursor-not-allowed"
		>
			{isSubmitting ? 'Sending...' : 'Send Message'}
		</button>
	</form>
</div>