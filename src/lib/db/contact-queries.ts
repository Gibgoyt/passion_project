import type { D1Database } from '@cloudflare/workers-types';

export interface ContactRecord {
  id: number;
  Name: string;
  Phone: string;
  Department: string;
  Message: string;
  RawEmail: string;
  CreatedAt: string;
}

export interface ContactFormData {
  name: string;
  phone: string;
  department?: string;
  message: string;
  rawEmail: string;
}

export const contactQueries = {
  create: (db: D1Database, data: ContactFormData) => {
    return db.prepare(`
      INSERT INTO ContactForm (Name, Phone, Department, Message, RawEmail)
      VALUES (?, ?, ?, ?, ?)
      RETURNING id, Name, Phone, Department, Message, RawEmail, CreatedAt
    `).bind(
      data.name,
      data.phone,
      data.department || '',
      data.message,
      data.rawEmail
    ).first<ContactRecord>();
  }
};
