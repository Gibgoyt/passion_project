import { type Validator, type ValidationResult, type ValidationError } from '../index';

export class SvelteForm<T extends Record<string, any>> {
    values = $state<T>({} as T);
    errors = $state<Record<keyof T, string>>({} as Record<keyof T, string>);
    touched = $state<Record<keyof T, boolean>>({} as Record<keyof T, boolean>);
    
    private schema: Record<keyof T, Validator<any>[]>;

    constructor(initialValues: T, schema: Record<keyof T, Validator<any>[]>) {
        this.values = { ...initialValues };
        this.schema = schema;
        
        // Initialize errors/touched
        Object.keys(initialValues).forEach(key => {
            (this.touched as any)[key] = false;
            (this.errors as any)[key] = '';
        });
    }

    validateField(field: keyof T) {
        const validators = this.schema[field];
        const value = this.values[field];
        
        for (const validator of validators) {
            const error = validator(value);
            if (error) {
                this.errors[field] = error;
                return false;
            }
        }
        
        this.errors[field] = '';
        return true;
    }

    validateAll(): boolean {
        let isValid = true;
        for (const key in this.schema) {
            if (!this.validateField(key)) {
                isValid = false;
            }
            this.touched[key] = true;
        }
        return isValid;
    }

    // Actions
    setField(field: keyof T, value: any) {
        this.values[field] = value;
        if (this.touched[field]) {
            this.validateField(field);
        }
    }

    blurField(field: keyof T) {
        this.touched[field] = true;
        this.validateField(field);
    }
    
    reset(newValues?: T) {
        if (newValues) {
             this.values = { ...newValues };
        }
        Object.keys(this.values).forEach(key => {
            (this.touched as any)[key] = false;
            (this.errors as any)[key] = '';
        });
    }

    get isValid() {
        // Check if any errors exist (we might need to run validation first if not touched, 
        // but for a getter, we usually rely on state)
        // A robust way is to check if the current error state is empty.
        // However, if fields haven't been touched/validated yet, we might want to know if it *would* be valid.
        // For simplicity in this optimized version, we assume 'errors' reflects the current known invalid state.
        // But to be safe for "submit button disabled" logic:
        return !Object.values(this.errors).some(e => e !== '');
    }
}
