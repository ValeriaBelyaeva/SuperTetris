# src/typescript_client/Dockerfile
# ---------- builder ----------
FROM node:18 AS builder
WORKDIR /app

COPY package*.json ./
RUN npm ci        # ставим зависимости по lock-файлу

# копируем все файлы проекта
COPY . .
RUN npm run build # выход → /app/dist

# ---------- runtime ----------
FROM nginx:stable-alpine
COPY --from=builder /app/dist /usr/share/nginx/html 