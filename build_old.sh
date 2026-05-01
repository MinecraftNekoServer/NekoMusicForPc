#!/bin/bash

# =========================================
# Neko云音乐 Electron 旧版本构建脚本
# =========================================

set -e  # 遇到错误立即退出

echo "========================================="
echo "  Neko云音乐 Electron Build Script"
echo "========================================="

# 检查是否在正确的目录
if [ ! -f "old/package.json" ]; then
    echo "错误：未找到 old/package.json"
    echo "请确保在项目根目录运行此脚本"
    exit 1
fi

# 进入旧版本目录
cd old

# 检查 Node.js 版本
NODE_VERSION=$(node -v | cut -d'v' -f2)
REQUIRED_NODE="20.19.0"
echo "Node.js 版本: $NODE_VERSION"

# 检查依赖是否已安装
if [ ! -d "node_modules" ]; then
    echo "未找到 node_modules，正在安装依赖..."
    npm install
fi

# 选择构建平台
echo ""
echo "请选择构建平台："
echo "1) Linux (.deb)"
echo "2) Windows (.exe)"
echo "3) macOS (.dmg)"
echo "4) 所有平台"
echo "5) 仅构建开发版本"
read -p "请输入选项 (1-5): " choice

case $choice in
    1)
        echo "构建 Linux 版本..."
        npm run build:linux
        ;;
    2)
        echo "构建 Windows 版本..."
        npm run build:win
        ;;
    3)
        echo "构建 macOS 版本..."
        npm run build:mac
        ;;
    4)
        echo "构建所有平台..."
        npm run build:all
        ;;
    5)
        echo "构建开发版本..."
        npm run build:dir
        ;;
    *)
        echo "无效选项，默认构建 Linux 版本..."
        npm run build:linux
        ;;
esac

echo ""
echo "========================================="
echo "  构建完成！"
echo "========================================="

# 显示构建结果
if [ -d "release" ]; then
    echo "构建产物位于: old/release/"
    ls -la release/
    
    # 如果是Linux构建，显示安装提示
    if [ "$choice" = "1" ] || [ "$choice" = "4" ]; then
        DEB_FILE=$(find release -name "*.deb" | head -1)
        if [ -n "$DEB_FILE" ]; then
            echo ""
            echo "安装命令:"
            echo "  sudo dpkg -i $DEB_FILE"
            echo "或:"
            echo "  sudo apt install ./$DEB_FILE"
        fi
    fi
fi

cd ..