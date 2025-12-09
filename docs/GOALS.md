# Single-Page Application Loading

[...all].astro creates GET /api/v1/spa_load, or some other bullshit, let's say this then returns the following:

```json
100   282  100   282    0     0  70376      0 --:--:-- --:--:-- --:--:-- 94000
[
  {
    "name": "Ahmed",
    "surname": "Moti",
    "createdAt": "2025-12-08T17:58:27Z",
    "updatedAt": "2025-12-09T17:14:46Z",
    "id": "Y7zw9KVWQAj9y8d9003ZGGloa2re"
  },
  {
    "name": "anand",
    "surname": "patel",
    "createdAt": "2025-12-08T09:22:41Z",
    "updatedAt": "2025-12-08T09:24:30Z",
    "id": "apgE4HtPD98BKI5fLSG5Mw4f6bns"
  }
]
```

To then load that as initialData={<initialData>} to the client as initial props

```typescript
interface User {
    name: string
    surname: string
    createdAt: string // or some ISO8601 type
    updatedAt: string // or some ISO8601 type
    id: string // or some UID type
}

interface InitialData = User[]
```

The goal for this being:

- Astro server then creates the HTTP request, and loads it with the entire single-page application
- HTTP request made on server (kinda like SSR, that the server fetches the data so the client has it immediately, but no rendering)
- all rendering is done on the client
