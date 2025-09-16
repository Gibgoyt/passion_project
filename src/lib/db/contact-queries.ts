import type { D1Database } from '@cloudflare/workers-types';

export interface ContactRecord {
  id: number;
  Name: string;
  Surname: string;
  RawPhoneNumber: string;
  ServiceInterest: string;
  Message: string;
}

export interface ContactFormData {
  name: string;
  phone: string;
  serviceInterest?: string;
  message: string;
}

export const contactQueries = {
  create: (db: D1Database, data: ContactFormData) => {
    // Split full name into Name and Surname
    const nameParts = data.name.trim().split(' ');
    const firstName = nameParts[0] || '';
    const lastName = nameParts.length > 1 ? nameParts.slice(1).join(' ') : '';

    return db.prepare(`
      INSERT INTO \`contact-form\` (Name, Surname, RawPhoneNumber, ServiceInterest, Message)
      VALUES (?, ?, ?, ?, ?)
      RETURNING id, Name, Surname, RawPhoneNumber, ServiceInterest, Message
    `).bind(
      firstName,
      lastName,
      data.phone,
      data.serviceInterest || '',
      data.message
    ).first<ContactRecord>();
  }
};