// Simple Runtime Validation Library

export type ValidationError = {
  field: string;
  message: string;
};

export type ValidationResult<T> =
  | { success: true; data: T }
  | { success: false; errors: ValidationError[] };

export type Validator<T> = (value: unknown) => string | null; // Returns error message or null

// Primitives
export const isString = (message = "Value must be a string"): Validator<string> => (value) => {
  return typeof value === "string" ? null : message;
};

export const required = (message = "This field is required"): Validator<any> => (value) => {
  if (value === null || value === undefined || value === "") return message;
  if (typeof value === "string" && value.trim() === "") return message;
  return null;
};

// String constraints
export const minLength = (min: number, message?: string): Validator<string> => (value) => {
  if (typeof value !== "string") return "Invalid type";
  return value.length >= min
    ? null
    : message || `Must be at least ${min} characters long`;
};

export const maxLength = (max: number, message?: string): Validator<string> => (value) => {
  if (typeof value !== "string") return "Invalid type";
  return value.length <= max
    ? null
    : message || `Must be at most ${max} characters long`;
};

export const pattern = (regex: RegExp, message: string): Validator<string> => (value) => {
  if (typeof value !== "string") return "Invalid type";
  return regex.test(value) ? null : message;
};

// Form Validator
export class FormValidator<T extends Record<string, any>> {
  private schema: Record<keyof T, Validator<any>[]>;

  constructor(schema: Record<keyof T, Validator<any>[]>) {
    this.schema = schema;
  }

  validate(data: Record<string, any>): ValidationResult<T> {
    const errors: ValidationError[] = [];
    const cleanData: any = {};

    for (const key in this.schema) {
      const value = data[key];
      const validators = this.schema[key];
      
      let fieldValid = true;
      for (const validator of validators) {
        const error = validator(value);
        if (error) {
          errors.push({ field: key, message: error });
          fieldValid = false;
          break; // Stop at first error per field
        }
      }
      
      if (fieldValid) {
        cleanData[key] = value;
      }
    }

    if (errors.length > 0) {
      return { success: false, errors };
    }

    return { success: true, data: cleanData as T };
  }
}
