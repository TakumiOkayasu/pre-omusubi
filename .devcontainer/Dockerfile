FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

ENV TZ=Asia/Tokyo \
    LANG=ja_JP.UTF-8 \
    LANGUAGE=ja_JP:jp \
    LC_ALL=ja_JP.UTF-8 \
    DEVCONTAINER=true \
    DEBIAN_FRONTEND=noninteractive

# Microsoft C++イメージには以下が既にインストール済み:
# bash, curl, git, sudo, ssh, unzip, ca-certificates
# build-essential, clang, cmake, ninja-build
# gdb, clang-format, clang-tidy, clangd
# 以下のパッケージのみ追加インストール
RUN apt-get update && apt-get install -y --no-install-recommends \
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

# terminal config - Git補完機能とカスタムプロンプトを追加
RUN curl -o /usr/share/bash-completion/completions/git-completion.bash \
        https://raw.githubusercontent.com/git/git/master/contrib/completion/git-completion.bash \
    && curl -o /usr/share/bash-completion/completions/git-prompt.sh \
        https://raw.githubusercontent.com/git/git/master/contrib/completion/git-prompt.sh \
    && chmod 755 /usr/share/bash-completion/completions/git-*.bash \
    && chmod 755 /usr/share/bash-completion/completions/git-*.sh \
    # .bashrcに追記（上書きしない）
    && cat >> /home/vscode/.bashrc <<'EOF'

# Git補完機能とカスタムプロンプト
if [ -f /usr/share/bash-completion/completions/git-completion.bash ]; then
    source /usr/share/bash-completion/completions/git-completion.bash
fi
if [ -f /usr/share/bash-completion/completions/git-prompt.sh ]; then
    source /usr/share/bash-completion/completions/git-prompt.sh
    GIT_PS1_SHOWDIRTYSTATE=true
    GIT_PS1_SHOWUNTRACKEDFILES=true
    GIT_PS1_SHOWUPSTREAM=auto
fi
EOF

WORKDIR /workspace
SHELL ["/bin/bash", "-c"]
