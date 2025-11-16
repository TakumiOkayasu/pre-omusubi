FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

ARG USERNAME
ARG USER_UID
ARG USER_GID

ENV TZ=Asia/Tokyo \
    LANG=ja_JP.UTF-8 \
    LANGUAGE=ja_JP:jp \
    LC_ALL=ja_JP.UTF-8 \
    DEVCONTAINER=true \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    # フォーマッタ
    clang-format \
    # 日本語ロケールとタイムゾーン設定用
    locales \
    tzdata \
    # 追加のデバッグ・ビルドツール
    valgrind \
    pkg-config \
    bear \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && locale-gen ja_JP.UTF-8 \
    && localedef -f UTF-8 -i ja_JP ja_JP.UTF-8

# uvのインストール
RUN curl -LsSf https://astral.sh/uv/install.sh | sh \
    && mv /root/.local/bin/uv /usr/local/bin/uv \
    && chmod 755 /usr/local/bin/uv

# 開発環境セットアップ
COPY scripts/omusubi-dev.sh /tmp/omusubi-dev.sh
COPY scripts/setup.sh /tmp/setup.sh
RUN bash /tmp/setup.sh && rm /tmp/setup.sh /tmp/omusubi-dev.sh

# vscodeユーザーに切り替え
USER ${USERNAME}

# PlatformIOをvscodeユーザーでインストール
ENV PLATFORMIO_CORE_DIR=/home/${USERNAME}/.platformio
RUN uv tool install platformio

WORKDIR /workspace
SHELL ["/bin/bash", "-c"]
