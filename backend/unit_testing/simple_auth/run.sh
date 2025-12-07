#!/bin/bash

# surely there is a better way to check if we are running the script from the current dir
CURRENT_DIR="/Users/ahmed/Projects/Astro/passion_project/backend/unit_testing/simple_auth"

if [ ! $PWD == $CURRENT_DIR ]; then
	echo "are you sure you are running in the current dir??"
	exit 1
fi

# Parse command line arguments
FORCE_INIT=0
while [[ $# -gt 0 ]]; do
	case $1 in
		--force-init)
			FORCE_INIT=1
			echo "🔄 Forcing OAuth client initialization..."
			shift
			;;
		-h|--help)
			echo "Usage: $0 [options]"
			echo "Options:"
			echo "  --force-init    Force OAuth client initialization even if clients already exist"
			echo "  -h, --help      Show this help message"
			exit 0
			;;
		*)
			echo "Unknown option: $1"
			echo "Use --help for usage information"
			exit 1
			;;
	esac
done

# Check and generate RSA keys if needed
if [[ ! -f keys/private_key.pem || ! -f keys/public_key.pem ]]; then
	echo "🔐 Generating RSA key pair..."
	./server --generate-keys
	echo ""
fi

# Check and build OAuth client initializer if needed
if [[ ! -f init_oauth_client ]]; then
	echo "🔧 Building OAuth client initializer..."
	make init_oauth_client
	echo ""
fi

# Check if OAuth clients database needs initialization
# Use init_oauth_client --check-clients to properly check if clients exist
NEED_OAUTH_INIT=0

echo "📋 Checking OAuth client database status..."
if [[ $FORCE_INIT -eq 1 ]]; then
	echo "🔄 Force flag set, will reinitialize OAuth clients..."
	NEED_OAUTH_INIT=1
elif ./init_oauth_client --check-clients 2>/dev/null; then
	echo "✅ OAuth clients already initialized"
else
	echo "📋 OAuth clients need initialization..."
	NEED_OAUTH_INIT=1
fi

# Initialize OAuth clients if needed
if [[ $NEED_OAUTH_INIT -eq 1 ]]; then
	echo "🚀 Initializing OAuth test clients..."
	if ./init_oauth_client; then
		echo "✅ OAuth client initialization completed successfully"

		# Verify initialization worked
		if ./init_oauth_client --check-clients 2>/dev/null; then
			echo "✅ Verification: All OAuth clients properly initialized"
		else
			echo "⚠️  Warning: OAuth client verification failed after initialization"
		fi
	else
		echo "❌ OAuth client initialization failed"
		echo "💡 You may need to run './init_oauth_client' manually"
		echo "   or use './run.sh --force-init' to retry"
		exit 1
	fi
	echo ""
fi

echo "🔄 Starting authentication server..."
./server
