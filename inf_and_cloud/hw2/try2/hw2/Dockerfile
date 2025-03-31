# Stage 1: Build Hugo from source
FROM golang:1.21-alpine AS hugo-builder


# Install git 
RUN apk add --no-cache git

# Clone Hugo
WORKDIR /src
RUN git clone --depth 1 --branch v0.111.3 https://github.com/gohugoio/hugo.git && \
    cd hugo && \
    go build -o hugo






# Stage 2: Create website
FROM alpine:3.19 AS site-builder


# Copy Hugo binary
COPY --from=hugo-builder /src/hugo/hugo /usr/local/bin/hugo

# Install git
RUN apk add --no-cache git



# Create Hugo site
WORKDIR /site
RUN hugo new site . --force && \
    mkdir -p content/about/ && \
    mkdir -p layouts/_default layouts/partials && \
    
    echo '<!DOCTYPE html><html><head><title>{{ .Title }}</title></head><body>{{ block "main" . }}{{ end }}</body></html>' > layouts/_default/baseof.html && \
    echo '{{ define "main" }}{{ if eq .Title "About" }}UCO: 570943{{ else }}<h1>{{ .Title }}</h1>{{ .Content }}{{ end }}{{ end }}' > layouts/_default/single.html && \
    echo '{{ define "main" }}<h1>{{ .Title }}</h1>{{ .Content }}{{ end }}' > layouts/_default/list.html


    COPY content/ content/


# Build the static site
RUN hugo --minify





# Stage 3: Nginx to serve the site
FROM alpine:3.19



# Install nginx
RUN apk add --no-cache nginx && \
    mkdir -p /run/nginx

# Add non-privileged user
RUN adduser -D -u 1000 www && \
    chown -R www:www /var/lib/nginx /run/nginx

# Configure nginx for non-root
RUN sed -i 's/user nginx;/user www;/' /etc/nginx/nginx.conf && \
    sed -i 's/\/var\/log\/nginx/\/tmp\/nginx/g' /etc/nginx/nginx.conf && \
    mkdir -p /tmp/nginx && \
    chown -R www:www /tmp/nginx



# Simple server config
RUN echo 'server { \
    listen 8080; \
    location / { \
        root /var/www/html; \
        index index.html; \
    } \
}' > /etc/nginx/http.d/default.conf

# Create and set permissions
RUN mkdir -p /var/www/html && \
    chown -R www:www /var/www/html

# Copy the built site
COPY --from=site-builder --chown=www:www /site/public/ /var/www/html/


# Switch to non-root user
USER www

# Expose port 8080
EXPOSE 8080


# Start Nginx
CMD ["nginx", "-g", "daemon off;"]