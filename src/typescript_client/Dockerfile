# src/typescript_client/Dockerfile
# ---------- builder ----------
FROM node:18 AS builder
WORKDIR /app

COPY package*.json ./
RUN npm ci        # ставим зависимости по lock-файлу

COPY public ./public
COPY src    ./src
RUN npm run build             # выход → /app/dist

# ---------- runtime ----------
FROM nginx:stable-alpine
COPY --from=builder /app/dist /usr/share/nginx/html 