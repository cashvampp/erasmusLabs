# Stage 1: Build the Hugo site
FROM ghcr.io/gohugoio/hugo:latest AS builder

# Set the working directory
WORKDIR /src

# Copy the Hugo site files into the container
COPY . .

# Ensure the permissions are correct (by using root)
USER root

# Build the Hugo site
RUN hugo

# Stage 2: Serve with Nginx
FROM nginx:alpine

# Copy custom nginx.conf
COPY nginx.conf /etc/nginx/nginx.conf

# Create necessary directories and adjust permissions
RUN mkdir -p /var/cache/nginx /var/run /var/log/nginx && \
    chown -R nginx:nginx /var/cache/nginx /var/run /var/log/nginx /usr/share/nginx/html

# Copy the built site from the builder stage
COPY --from=builder /src/public /usr/share/nginx/html

# Expose port 8080
EXPOSE 8080

# Command to run Nginx as a non-root user
USER nginx

# Command to run Nginx
CMD ["nginx", "-g", "daemon off;"]

#at this point im just depressed, i`ve got 47,9mb container and cant go lower, it broke several times while i tried to, i`m just happy it works now (i hope)
