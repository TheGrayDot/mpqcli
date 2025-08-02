# Stage 1: Build and Test
FROM alpine:3.22 AS builder

RUN apk add --no-cache \
    build-base \
    cmake \
    git \
    python3 \
    py3-pip \
    libstdc++ \
    libgcc

WORKDIR /mpqcli

COPY . .

RUN cmake -B build

RUN cmake --build build

RUN pip install --no-cache-dir -r test/requirements.txt

RUN python3 -m pytest test

# Stage 2: Create a minimal runtime image
FROM alpine:3.22 AS runtime

RUN apk add --no-cache \
    libstdc++ \
    libgcc

WORKDIR /mpqcli

COPY --from=builder /mpqcli/build/bin/mpqcli /usr/local/bin/mpqcli

RUN chmod +x /usr/local/bin/mpqcli

ENTRYPOINT ["mpqcli"]
