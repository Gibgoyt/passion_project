/// <reference path="../.astro/types.d.ts" />

declare namespace App {
  interface Locals {
    user?: {
      userId: string
      email: string
      emailVerified: boolean
    }
    isAuthenticated: boolean
  }
}