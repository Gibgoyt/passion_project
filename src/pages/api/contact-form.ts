import type { APIRoute } from 'astro';
import { contactQueries, type ContactFormData } from 'src/lib/db/contact-queries';

export const POST: APIRoute = async ({ request, locals }) => {
  try {
    // Get D1 database instance
    const runtime = locals.runtime;
    const db = runtime?.env?.DB;

    if (!db) {
      return new Response(
        JSON.stringify({
          success: false,
          error: 'Database not configured'
        }),
        {
          status: 500,
          headers: {
            'Content-Type': 'application/json'
          }
        }
      );
    }

    // Parse form data
    let formData: ContactFormData;

    try {
      const body = await request.json();
      formData = {
        name: body.name?.trim(),
        phone: body.phone?.trim(),
        department: body.service?.trim(),
        message: body.message?.trim(),
        rawEmail: "placeholder@example.com" // Hardcoded as form doesn't provide it
      };
    } catch (error) {
      return new Response(
        JSON.stringify({
          success: false,
          error: 'Invalid request format'
        }),
        {
          status: 400,
          headers: {
            'Content-Type': 'application/json'
          }
        }
      );
    }

    // Validate required fields
    if (!formData.name || !formData.phone || !formData.message) {
      return new Response(
        JSON.stringify({
          success: false,
          error: 'Missing required fields: name, phone, and message are required'
        }),
        {
          status: 400,
          headers: {
            'Content-Type': 'application/json'
          }
        }
      );
    }

    // Validate field lengths
    if (formData.name.length > 100) {
      return new Response(
        JSON.stringify({
          success: false,
          error: 'Name must be 100 characters or less'
        }),
        {
          status: 400,
          headers: {
            'Content-Type': 'application/json'
          }
        }
      );
    }

    if (formData.phone.length > 20) {
      return new Response(
        JSON.stringify({
          success: false,
          error: 'Phone number must be 20 characters or less'
        }),
        {
          status: 400,
          headers: {
            'Content-Type': 'application/json'
          }
        }
      );
    }

    if (formData.message.length > 1000) {
      return new Response(
        JSON.stringify({
          success: false,
          error: 'Message must be 1000 characters or less'
        }),
        {
          status: 400,
          headers: {
            'Content-Type': 'application/json'
          }
        }
      );
    }

    // Insert into database
    const result = await contactQueries.create(db, formData);

    if (!result) {
      return new Response(
        JSON.stringify({
          success: false,
          error: 'Failed to save contact form'
        }),
        {
          status: 500,
          headers: {
            'Content-Type': 'application/json'
          }
        }
      );
    }

    // Success response
    return new Response(
      JSON.stringify({
        success: true,
        message: 'Thank you for your message! We will get back to you soon.',
        id: result.id
      }),
      {
        status: 200,
        headers: {
          'Content-Type': 'application/json'
        }
      }
    );

  } catch (error) {
    console.error('Contact form API error:', error);

    return new Response(
      JSON.stringify({
        success: false,
        error: 'An unexpected error occurred. Please try again later.'
      }),
      {
        status: 500,
        headers: {
          'Content-Type': 'application/json'
        }
      }
    );
  }
};
